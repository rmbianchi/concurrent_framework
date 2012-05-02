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
//#include "tbb/flow_graph.h"
#include "tbb/compat/thread"
//include fwk
#include "Algo.h"
#include "AlgoPool.h"
#include "Context.h"
#include "Sequence.h"


//typedef uint64_t state_type;
typedef std::bitset<1000> state_type;


// forward declarations
class Scheduler;
class EventLoopManager;
class EventState;



/**
 * The AlgoTaskId the item used for call back once tbb finished the AlgoTask.
 * It combines the pointer to the algorithm/module with the EventState
 */
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


/**
 * The class storing the state of an event and the states of the 
 * algorithms/modules for that event.
 */
class EventState{
public:
    EventState(unsigned int algos) : state(0), context(0), algo_states(algos,NOT_RUN){};
    ~EventState(){};
    state_type state;
    Context* context;
    std::vector<AlgoState> algo_states;
};


/**
 * The class which schedules and handles all the tasks for algorithms/modules
 */
class Scheduler {
public:
    Scheduler(Whiteboard& wb);
    void task_cleanup();
    void algo_is_done(AlgoTaskId* task_id);
    void start_event(unsigned int event);
    void initialise(AlgoPool* algo_pool, const std::vector<AlgoBase*>* algos, EventLoopManager* looper);
    void stop(){has_to_stop_ = true;};
    void operator() ();
private:
    std::vector<state_type> compute_dependencies();
    const std::vector<AlgoBase*>* algos_;
    AlgoPool* algo_pool_; 
    state_type termination_requirement_;
    Whiteboard& wb_;
    tbb::concurrent_queue<AlgoTaskId*> done_queue_;
    tbb::concurrent_queue<unsigned int> new_events_queue_;
    EventLoopManager* loop_manager_;
    tbb::atomic<bool> has_to_stop_;
};


/**
 * The class handling the task for the main scheduler.
 */
class SchedulerTask : public tbb::task {
    public:    
        SchedulerTask(Scheduler* scheduler): scheduler_(scheduler){};    
        tbb::task* execute(){(*scheduler_)();return NULL;};
    private:    
        Scheduler* scheduler_;
    
    };

#endif
