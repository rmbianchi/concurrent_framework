//
//  Scheduler.cpp
//  
//
//  Created by Benedikt Hegner on 4/10/12.
//  Copyright (c) 2012 __CERN__. All rights reserved.
//

#include <iostream>

#include "Scheduler.h"

tbb::task* AlgoTask::execute(){
    task_->algo_->body(task_->context_);
    done_queue_->push(task_);
    return NULL;
}


Scheduler::Scheduler(const std::vector<AlgoBase*>& algorithms, Whiteboard& wb, unsigned int max_concurrent_events) : algos_(algorithms), wb_(wb), max_concurrent_events_(max_concurrent_events) {
    // put in a termination algo
    algos_.push_back(new EndAlgo("*END*"));
    // Fill the data structure holding all available algorithm instances
    const unsigned int size = algos_.size();
    available_algo_instances_.resize(size);
    for (unsigned int i = 0; i<size; ++i) {
        available_algo_instances_[i] = new tbb::concurrent_queue<AlgoBase*>();   
        available_algo_instances_[i]->push(algos_[i]);
    }
}  


std::vector<unsigned int> Scheduler::compute_dependencies() {
    std::vector<unsigned int> all_requirements(algos_.size());
    // create the mapping productname : index
    std::map<std::string,unsigned int> product_indices;
    for (unsigned int i = 0, n_algos = algos_.size(); i < n_algos; ++i) {
        AlgoBase* algo = algos_[i];
        const std::vector<std::string>& outputs = algo->get_outputs();
        for (unsigned int j = 0, n_outputs = outputs.size(); j < n_outputs; ++j){
            product_indices[outputs[j]] = i;
        }
    }
    // use the mapping to create a bit pattern of input requirements
    unsigned int termination_requirement = 0;
    for (unsigned int i = 0, n_algos = algos_.size(); i < n_algos; ++i) {
        unsigned int requirements = 0;
        printf(" %i: %s\n",i,algos_[i]->get_name());
        const std::vector<std::string>& inputs = algos_[i]->get_inputs();
        for (unsigned int j = 0, n_inputs = inputs.size(); j < n_inputs; ++j){
            unsigned int input_index = product_indices[inputs[j]];
            requirements = requirements | (1 << input_index);
            printf("\tconnecting to %s (via '%s')\n", algos_[input_index]->get_name(), inputs[j].c_str());
        }
        all_requirements[i] = requirements;
        termination_requirement = termination_requirement | (1 << i);
    }
    all_requirements[(algos_.size()-1)] = termination_requirement >> 1; // the endalgo has to wait for everything else, except for itself
    return all_requirements;  
}


// Scheduler using a bit mask for analysis of what can be run;
void Scheduler::run_parallel(int n){
    printf("++++++++++++++++++++++++++++\n");
    printf(" Using scheduler flavour #2\n");
    printf("++++++++++++++++++++++++++++\n");
    std::vector<std::pair<unsigned int, Context*> > event_states(max_concurrent_events_);
    //get the bit patterns and sort by node id (like the available algos)
    std::vector<unsigned int> bits = compute_dependencies();   
    // some book keeping vectors
    size_t size = algos_.size();
    std::vector<std::vector<bool> > algo_has_run_in_eventid(max_concurrent_events_, std::vector<bool>(size, false)); //TODO: replace     
    unsigned int in_flight(0), processed(0), current_event(0);  
    
    do {        
        // check if a new event can and should be started
        if (in_flight < max_concurrent_events_ && processed+in_flight < n) {
            for (unsigned int i=0, max = event_states.size() ; i<max; ++i) {
                if (event_states[i].second==NULL){ 
                    //since the pointer to the Context is NULL, this slot is free
                    // is the whiteboard available for this event?
                    Context* context(0);
                    bool whiteboard_available = wb_.get_context(context);
                    if (whiteboard_available){
                        algo_has_run_in_eventid[i]=std::vector<bool>(size, false);
                        event_states[i].first = 0;
                        event_states[i].second = context;
                        event_states[i].second->write(processed+in_flight, "event","event");
                        ++current_event;
                        ++in_flight;  
                        break;
                    } else {
                        printf("no whiteboard available\n");
                    }
                }
            } 
        }
        
        // now schedule whatever can be scheduled
        // loop through the entire vector of algo bits
        for (unsigned int algo = 0; algo < size; ++algo) {
            // loop through all currently active events
            for (unsigned int event_id = 0; event_id < max_concurrent_events_ ; ++event_id) {
                if (event_states[event_id].second != NULL) {
                    // extract event_id specific quantities
                    unsigned int& current_event_bits = event_states[event_id].first;
                    // check whether all dependencies for the algorithm are fulfilled...
                    unsigned int tmp = (current_event_bits & bits[algo]) ^ bits[algo];
                    // ... and whether the algo was previously started
                    std::vector<bool>& algo_has_run = algo_has_run_in_eventid[event_id];
                    if ((tmp==0) && (algo_has_run[algo] == false)) {
                        // is there an available Algo instance one can use?
                        AlgoBase* algo_instance(0);
                        bool algo_free(0);
                        algo_free = available_algo_instances_[algo]->try_pop(algo_instance);
                        if (algo_free) { 
                            Context*& context = event_states[event_id].second;
                            AlgoTaskId* task = new AlgoTaskId(algos_[algo],algo,event_id,context);    
                            tbb::task* t = new( tbb::task::allocate_root() ) AlgoTask(task, &done_queue_);
                            tbb::task::enqueue( *t); 
                            algo_has_run[algo] = true;
                        }
                    }
                }
            }
        }  
        
        task_cleanup(event_states);
        
        // check for finished events and clean up
        for (unsigned int i=0, max = event_states.size() ; i<max; ++i) {
            Context*& context = event_states[i].second;
            if (context != NULL) {
                if (context->is_finished()) {
                    printf("Finished event\n"); 
                    ++processed; 
                    --in_flight;
                    wb_.release_context(context);
                    context = NULL; 
                } 
            }
        }     
    } while (processed < n);
    
}


// check for finished tasks, free the used algo instances and delete the AlgoTaskId
// TODO: Much of it could be moved into the call back procedure of AlgoTask
//       once we made all the state objects thread safe
void Scheduler::task_cleanup(std::vector<std::pair<unsigned int, Context*> >& event_states){    
    AlgoTaskId* result(0);
    bool queue_full(false);
    do {
        queue_full = done_queue_.try_pop(result);
        if (queue_full) {
            available_algo_instances_[result->algo_id_]->push(result->algo_);
            unsigned int old_bits(event_states[result->event_id_].first); 
            unsigned int new_bits = old_bits | (1 << result->algo_id_);
            event_states[result->event_id_].first = new_bits;
            delete result;
        }
    } while (queue_full);  
}

