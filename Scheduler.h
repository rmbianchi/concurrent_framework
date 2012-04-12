//
//  Scheduler.h
//  
//
//  Created by Benedikt Hegner on 4/10/12.
//  Copyright (c) 2012 __CERN__. All rights reserved.
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

class Scheduler;

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
 * it specifically gets the done_queue for talk back
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
    EventState(unsigned int algos) : state(0), context(0), algos_started_(algos,false), algos_passed_(algos,false){};
    ~EventState(){};
    state_type state;
    Context* context;
    tbb::concurrent_vector<bool> algos_started_;
    tbb::concurrent_vector<bool> algos_passed_;
};

class Scheduler {
public:
    Scheduler(Whiteboard& wb, unsigned int max_concurrent_events, AlgoPool& algo_pool, const std::vector<AlgoBase*>& algos);
    void run_parallel(int n);
    void task_cleanup();
    void algo_is_done(AlgoTaskId* task_id);
    void start_event(unsigned int event);
private:
    std::vector<state_type> compute_dependencies();
    const std::vector<AlgoBase*>& algos_;
    state_type termination_requirement_;
    Whiteboard& wb_;
    unsigned int max_concurrent_events_; //to be removed
    tbb::concurrent_queue<AlgoTaskId*> done_queue_;
    AlgoPool& algo_pool_; 
    tbb::queuing_mutex task_callback_mutex_;
};


#endif
