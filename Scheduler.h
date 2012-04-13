//
//  Scheduler.h
//  
//
//  Created by Benedikt Hegner on 4/10/12.
//  Copyright (c) 2012 CERN. All rights reserved.
//

#ifndef _Scheduler_h
#define _Scheduler_h

// include c++
#include <map>
#include <vector>
#include <string>
#include <bitset>
// include tbb
#include "tbb/concurrent_queue.h"
#include "tbb/concurrent_vector.h"
#include "tbb/task.h"
//include fwk
#include "Algo.h"
#include "AlgoPool.h"
#include "Context.h"

//typedef uint64_t state_type;
typedef std::bitset<1000> state_type;

// forward declarations
class Scheduler;
class EventLoopManager;

/**
 * The AlgoTaskId the item used for call back once tbb finished the AlgoTask
 */
class EventState;

class AlgoTaskId {
public:
    AlgoTaskId(AlgoBase* algo, unsigned int algo_id, EventState* event_state): algo_(algo),algo_id_(algo_id), event_state_(event_state){};
    AlgoBase* algo_;
    unsigned int algo_id_;
    EventState* event_state_;
};

/**
 * The tbb::task implementation that gets passed to tbb::task::enqueue
 * as opposed to AlgoTaskId it can be disposed by tbb (as per design)
 */
class AlgoTask : public tbb::task {
public:
    AlgoTask(AlgoTaskId* task, Scheduler* scheduler): task_(task), scheduler_(scheduler){};    
    tbb::task* execute();
    AlgoTaskId* task_;
    Scheduler* scheduler_;
};


class EventState{
public:
    EventState(unsigned int algos) : state(0), context(0), algo_states(algos,NOT_RUN){};
    ~EventState(){};
    state_type state;
    Context* context;
    tbb::concurrent_vector<AlgoState> algo_states;
};

class Scheduler {
public:
    Scheduler(Whiteboard& wb, unsigned int max_concurrent_events, AlgoPool& algo_pool, const std::vector<AlgoBase*>& algos, EventLoopManager* looper);
    void task_cleanup();
    void algo_is_done(AlgoTaskId* task_id);
    void start_event(unsigned int event);
    void initialise();
    void stop(){has_to_stop_ = true;};
    void run();
private:
    std::vector<state_type> compute_dependencies();
    const std::vector<AlgoBase*>& algos_;
    state_type termination_requirement_;
    Whiteboard& wb_;
    tbb::concurrent_queue<AlgoTaskId*> done_queue_;
    tbb::concurrent_queue<unsigned int> new_events_queue_;
    AlgoPool& algo_pool_; 
    tbb::queuing_mutex task_callback_mutex_;
    EventLoopManager* event_loop_manager_;
    bool has_to_stop_; //TODO: thread safety
};

class SchedulerTask : public tbb::task {
public:    
    SchedulerTask(Scheduler* scheduler): scheduler_(scheduler){};    
    tbb::task* execute(){scheduler_->run();return NULL;};
private:    
    Scheduler* scheduler_;

};


#endif
