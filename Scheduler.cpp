//
//  Scheduler.cpp
//  
//
//  Created by Benedikt Hegner on 4/10/12.
//  Copyright (c) 2012 CERN. All rights reserved.
//

// include tbb
#include "tbb/compat/thread"
// include fwk
#include "EventLoopManager.h"
#include "Scheduler.h"


tbb::task* AlgoTask::execute(){
    bool successful = task_->algo_->body(task_->event_state_->context);
    scheduler_->algo_is_done(task_);
    //if (successful) task_->event_state_->algo_states[task_->algo_id_] = ACCEPT;
    //else task_->event_state_->algo_states[task_->algo_id_] = REJECT;
    return NULL;
}

Scheduler::Scheduler(Whiteboard& wb, unsigned int max_concurrent_events, AlgoPool& algo_pool, const std::vector<AlgoBase*>& algos, EventLoopManager* looper ) : 
    algos_(algos), wb_(wb), new_events_queue_(), algo_pool_(algo_pool), 
    event_loop_manager_(looper), has_to_stop_(false)
{
}  


std::vector<state_type> Scheduler::compute_dependencies() {
    std::vector<state_type> all_requirements(algos_.size());
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
    state_type termination_requirement(0);
    for (unsigned int i = 0, n_algos = algos_.size(); i < n_algos; ++i) {
        state_type requirements(0);
        //printf(" %i: %s\n",i,algos_[i]->get_name());
        const std::vector<std::string>& inputs = algos_[i]->get_inputs();
        for (unsigned int j = 0, n_inputs = inputs.size(); j < n_inputs; ++j){
            unsigned int input_index = product_indices[inputs[j]];
            requirements[input_index] = true;
            //printf("\tconnecting to %s (via '%s')\n", algos_[input_index]->get_name(), inputs[j].c_str());
        }
        all_requirements[i] = requirements;
        termination_requirement[i] = true;
    }
    termination_requirement_ = termination_requirement;
    return all_requirements;  
}

// check for finished tasks, free the used algo instances and delete the AlgoTaskId
void Scheduler::task_cleanup(){    
    AlgoTaskId* result(0);
    bool queue_full(false);
    do {
        queue_full = done_queue_.try_pop(result);
        if (queue_full) {
            algo_pool_.release(result->algo_, result->algo_id_);
            state_type new_bits(result->event_state_->state); 
            new_bits[result->algo_id_] = true;
            result->event_state_->state = new_bits;
            result->event_state_->algo_states[result->algo_id_] = ACCEPT;
            delete result;
        }
    } while (queue_full);  
}

void Scheduler::algo_is_done(AlgoTaskId* result){
    //algo_pool_.release(result->algo_, result->algo_id_);
    //state_type new_bits(result->event_state_->state); 
    //new_bits[result->algo_id_] = true;
    //tbb::queuing_mutex::scoped_lock lock;
    //lock.acquire(task_callback_mutex_);
    //result->event_state_->state = new_bits;
    //lock.release();    
    //delete result;
    
    done_queue_.push(result);
}

void Scheduler::start_event(unsigned int event_number){
    new_events_queue_.push(event_number);
}

void Scheduler::initialise(){}

void Scheduler::run(){
    //get the bit patterns and sort by node id (like the available algos)
    std::vector<state_type> bits = compute_dependencies();   
    // some book keeping vectors
    size_t size = algos_.size();
    std::vector<EventState*> event_states(0); //TODO - has to move to init 
    do {        
        // BEGIN TODO: replace by thread safe code in start event
        // loop through all events that need to be started
        bool queue_full(false);
        do {
            unsigned int event_number(0);
            queue_full = new_events_queue_.try_pop(event_number);
            if (queue_full){
                Context* context(0);
                bool whiteboard_available = wb_.get_context(context);
                if (whiteboard_available){
                    EventState* event_state = new EventState(size);
                    event_states.push_back(event_state);
                    event_state->context = context;
                    context->write(event_number, "event","event");
                } 
            }
        } while(queue_full);

        
        for (unsigned int algo = 0; algo < size; ++algo) {
            // loop through all currently active events
            for (unsigned int event_id = 0; event_id < event_states.size() ; ++event_id) {
                EventState*& event_state = event_states[event_id];
                // extract event_id specific quantities
                state_type& current_event_bits = event_state->state;
                // check whether all dependencies for the algorithm are fulfilled...
                state_type tmp = (current_event_bits & bits[algo]) ^ bits[algo];
                /// ...whether all required products are there...
                
                // ... and whether the algo was previously started
                std::vector<AlgoState>& algo_states = event_state->algo_states;
                if ((tmp==0) && (algo_states[algo] == NOT_RUN)) {
                    // is there an available Algo instance one can use?
                    AlgoBase* algo_instance(0);
                    bool algo_free(0);
                    algo_free = algo_pool_.acquire(algo_instance, algo);
                    if (algo_free) { ; 
                        AlgoTaskId* task = new AlgoTaskId(algos_[algo],algo,event_state);    
                        tbb::task* t = new( tbb::task::allocate_root() ) AlgoTask(task, this);
                        tbb::task::enqueue( *t); 
                        algo_states[algo] = SCHEDULED;
                    }
                }
            }
        }
        
        task_cleanup();
        
        // check for finished events and clean up
        for (std::vector<EventState*>::iterator i = event_states.begin(), end = event_states.end(); i != end; ++i){
            if ((*i)->state == termination_requirement_) {
                Context*& context = (*i)->context;
                wb_.release_context(context);
                event_loop_manager_->finished_event();
                delete (*i);
                i = event_states.erase(i);
            }
        }

        
        std::this_thread::yield();  
        
        
    } while (not has_to_stop_);
    return;
};

