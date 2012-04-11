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
    termination_requirement_ = termination_requirement;
    return all_requirements;  
}

// Scheduler using a bit mask for analysis of what can be run;
void Scheduler::run_parallel(int n){
    printf("++++++++++++++++++++++++++++\n");
    printf(" Using scheduler flavour #2\n");
    printf("++++++++++++++++++++++++++++\n");
    //get the bit patterns and sort by node id (like the available algos)
    std::vector<unsigned int> bits = compute_dependencies();   
    // some book keeping vectors
    size_t size = algos_.size();
    std::vector<EventState*> event_states(0);
    unsigned int in_flight(0), processed(0), current_event(0);  
    
    do {        
        // check if a new event can and should be started
        if (in_flight < max_concurrent_events_ && processed+in_flight < n) {
            Context* context(0);
            bool whiteboard_available = wb_.get_context(context);
            if (whiteboard_available){
                EventState* event_state = new EventState(size);
                event_states.push_back(event_state);
                event_state->started_algos=std::vector<bool>(size, false);
                event_state->state = 0;
                event_state->context = context;
                event_state->context->write(processed+in_flight, "event","event");
                ++current_event;
                ++in_flight;  
            } else {
                printf("no whiteboard available\n");
            }
        } 
        // now schedule whatever can be scheduled
        // loop through the entire vector of algo bits
        for (unsigned int algo = 0; algo < size; ++algo) {
            // loop through all currently active events
            for (unsigned int event_id = 0; event_id < event_states.size() ; ++event_id) {
                EventState*& event_state = event_states[event_id];
                // extract event_id specific quantities
                unsigned int& current_event_bits = event_state->state;
                // check whether all dependencies for the algorithm are fulfilled...
                unsigned int tmp = (current_event_bits & bits[algo]) ^ bits[algo];
                // ... and whether the algo was previously started
                std::vector<bool>& algo_has_run = event_state->started_algos;
                if ((tmp==0) && (algo_has_run[algo] == false)) {
                    // is there an available Algo instance one can use?
                    AlgoBase* algo_instance(0);
                    bool algo_free(0);
                    algo_free = available_algo_instances_[algo]->try_pop(algo_instance);
                    if (algo_free) { 
                        Context*& context = event_state->context;
                        AlgoTaskId* task = new AlgoTaskId(algos_[algo],algo,event_state,context);    
                        tbb::task* t = new( tbb::task::allocate_root() ) AlgoTask(task, &done_queue_);
                        tbb::task::enqueue( *t); 
                        algo_has_run[algo] = true;
                    }
                }
            }
        }  
        
        task_cleanup(event_states);
        
        // check for finished events and clean up
        for (std::vector<EventState*>::iterator i = event_states.begin(), end = event_states.end(); i != end; ++i){
            if ((*i)->state == termination_requirement_) {
                Context*& context = (*i)->context;
                printf("Finished event\n"); 
                wb_.release_context(context);
                ++processed; 
                --in_flight;
                delete (*i);
                i = event_states.erase(i);
                break;
            }
        }
    } while (processed < n);
}


// check for finished tasks, free the used algo instances and delete the AlgoTaskId
// TODO: Much of it could be moved into the call back procedure of AlgoTask
//       once we made all the state objects thread safe
void Scheduler::task_cleanup(std::vector<EventState*>& event_states){    
    AlgoTaskId* result(0);
    bool queue_full(false);
    do {
        queue_full = done_queue_.try_pop(result);
        if (queue_full) {
            available_algo_instances_[result->algo_id_]->push(result->algo_);
            unsigned int old_bits(result->event_state_->state); 
            unsigned int new_bits = old_bits | (1 << result->algo_id_);
            result->event_state_->state = new_bits;
            delete result;
        }
    } while (queue_full);  
}

