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
    m_task->algo->body(m_task->context);
    m_done_queue->push(m_task);
    return NULL;
}

std::vector<unsigned int> Scheduler::compute_dependencies(){
    std::vector<unsigned int> all_requirements(m_algos.size());
    // create the mapping productname : index
    std::map<std::string,unsigned int> product_indices;
    for (unsigned int i = 0, n_algos = m_algos.size(); i < n_algos; ++i) {
        AlgoBase* algo = m_algos[i];
        const std::vector<std::string>& outputs = algo->get_outputs();
        for (unsigned int j = 0, n_outputs = outputs.size(); j < n_outputs; ++j){
            product_indices[outputs[j]] = i;
        }
    }
    unsigned int termination_requirement = 0;
    // use the mapping to create a bit pattern of input requirements
    for (unsigned int i = 0, n_algos = m_algos.size(); i < n_algos; ++i) {
        unsigned int requirements = 0;
        printf(" %i: %s\n",i,m_algos[i]->get_name());
        const std::vector<std::string>& inputs = m_algos[i]->get_inputs();
        for (unsigned int j = 0, n_inputs = inputs.size(); j < n_inputs; ++j){
            unsigned int input_index = product_indices[inputs[j]];
            requirements = requirements | (1 << input_index);
            printf("\tconnecting to %s (via '%s')\n", m_algos[input_index]->get_name(), inputs[j].c_str());
        }
        all_requirements[i] = requirements;
        termination_requirement = termination_requirement | (1 << i);
    }
    all_requirements[(m_algos.size()-1)] = termination_requirement >> 1; // the endalgo has to wait for everything else, except for itself
    return all_requirements;  
}



Scheduler::Scheduler(const std::vector<AlgoBase*>& algorithms, Whiteboard& wb, unsigned int max_concurrent_events) : m_algos(algorithms), m_wb(wb), m_max_concurrent_events(max_concurrent_events){
    // Fill the data structure holding all available algorithm instances
    //TODO: make this configurable; requires proper copy constructor of algos

    // put in a termination algo
    m_algos.push_back(new EndAlgo("*END*"));
    const unsigned int size = m_algos.size();
    m_available_algo_instances.resize(size);
    for (unsigned int i = 0; i<size; ++i) {
        m_available_algo_instances[i] = new tbb::concurrent_queue<AlgoBase*>();   
        m_available_algo_instances[i]->push(m_algos[i]);
    }
}  


// Scheduler using a bit mask for analysis of what can be run;
void Scheduler::run_parallel(int n){
    printf("++++++++++++++++++++++++++++\n");
    printf(" Using scheduler flavour #2\n");
    printf("++++++++++++++++++++++++++++\n");
    std::vector<std::pair<unsigned int, Context*> > bit_events(m_max_concurrent_events);
    //get the bit patterns and sort by node id (like the available algos)
    size_t size = m_algos.size();
    // some book keeping vectors
    std::vector<std::vector<bool> > algo_has_run_in_eventid(m_max_concurrent_events, std::vector<bool>(size, false)); //TODO: replace   
    std::vector<unsigned int> bits = compute_dependencies(); 
    
    unsigned int in_flight(0), processed(0);
    unsigned int current_event(0);  
    do {        
        // check if a new event can and should be started
        int available_eventid = -1;
        if (in_flight < m_max_concurrent_events && processed+in_flight < n) {
            for (unsigned int i=0, max = bit_events.size() ; i<max; ++i) {
                if (bit_events[i].second==NULL){ 
                    //since the pointer to the Context is NULL, this slot is free
                    available_eventid = i;
                    // is the whiteboard available for this event?
                    Context* context(0);
                    bool whiteboard_available = m_wb.get_context(context); //TODO
                    int& i = available_eventid;
                    bit_events[i].first = 0;
                    bit_events[i].second = context;
                    bit_events[i].second->write(processed+in_flight, "event","event");
                    ++current_event;
                    ++in_flight;  
                    break;
                }
            } 
        }
        
        // now schedule whatever can be scheduled
        // loop through the entire vector of algo bits
        for (unsigned int algo = 0; algo < size; ++algo) {
            // loop through all currently active events
            for (unsigned int event_id = 0; event_id < m_max_concurrent_events ; ++event_id) {
                if (bit_events[event_id].second != NULL) {
                    // extract event_id specific quantities
                    unsigned int& current_event_bits = bit_events[event_id].first;
                    Context*& context = bit_events[event_id].second;
                    std::vector<bool>& algo_has_run = algo_has_run_in_eventid[event_id];
                    // check bit pattern
                    unsigned int tmp = (current_event_bits & bits[algo]) ^ bits[algo];
                    AlgoBase* algo_instance(0);
                    bool algo_free(0);
                    if ((tmp==0) && (algo_has_run[algo] == false)) {
                        algo_free = m_available_algo_instances[algo]->try_pop(algo_instance);
                        if (algo_free) { 
                            AlgoTaskId* task = new AlgoTaskId(m_algos[algo],algo,event_id,context);    
                            tbb::task* t = new( tbb::task::allocate_root() ) AlgoTask(task, &m_bits_done_queue);
                            tbb::task::enqueue( *t); 
                            algo_has_run[algo] = true;
                        }
                    }
                }
            }
        }  
        
        // check for finished tasks
        // free the used algo instances
        // update the set bits for the particular event
        // then delete the TaskItemId        
        AlgoTaskId* result(0);
        bool queue_full(false);
        do {
            queue_full = m_bits_done_queue.try_pop(result);
            if (queue_full) {
                m_available_algo_instances[result->algo_id]->push(result->algo);
                unsigned int old_bits(bit_events[result->event_id].first); 
                unsigned int new_bits = old_bits | (1 << result->algo_id);
                bit_events[result->event_id].first = new_bits;
                delete result;
            }
        } while (queue_full);  
        
        for (unsigned int i=0, max = bit_events.size() ; i<max; ++i) {
            if (bit_events[i].second != NULL) {
                if (bit_events[i].second->is_finished()) {
                    ++processed; 
                    --in_flight; 
                    bit_events[i].second = NULL; 
                    printf("Finished event\n"); } // TODO: add proper context disconnect; free used algo vector
            }
        }     
    } while (processed < n);
    
}

void Scheduler::task_cleanup(){
    
}

