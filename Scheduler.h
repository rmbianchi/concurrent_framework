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
// include tbb
#include "tbb/concurrent_queue.h"
#include "tbb/concurrent_vector.h"
#include "tbb/task.h"
#include "tbb/atomic.h"
//include fwk
#include "Algo.h"
#include "Context.h"

/**
 * The AlgoTaskId the item used for call back once tbb finished the AlgoTask
 */
class AlgoTaskId {
public:
    AlgoTaskId(AlgoBase* algo, unsigned int algo_id, unsigned int event_id, Context* context): algo_(algo),algo_id_(algo_id), event_id_(event_id), context_(context) {};
    AlgoBase* algo_;
    unsigned int algo_id_;
    unsigned int event_id_;
    Context* context_;
};

/**
 * The tbb::task implementation that gets passed to tbb::task::enqueue
 * as opposed to AlgoTaskId it can be disposed by tbb (as per design)
 * it specifically gets the done_queue for talk back
 */
class AlgoTask : public tbb::task {
public:
    AlgoTask(AlgoTaskId* task, tbb::concurrent_queue<AlgoTaskId*>* done_queue): task_(task), done_queue_(done_queue){};    
    tbb::task* execute();
    AlgoTaskId* task_;
    tbb::concurrent_queue<AlgoTaskId*>* done_queue_;
};


class Scheduler {
public:
    Scheduler(const std::vector<AlgoBase*>& algorithms, Whiteboard& wb, unsigned int max_concurrent_events);
    void run_parallel(int n);
    void task_cleanup(std::vector<std::pair<unsigned int, Context*> >& event_states);
private:
    std::vector<unsigned int> compute_dependencies();
    std::vector<AlgoBase*> algos_;
    Whiteboard& wb_;
    unsigned int max_concurrent_events_;
    std::vector<tbb::concurrent_queue<AlgoBase*>*> available_algo_instances_;
    tbb::concurrent_queue<AlgoTaskId*> done_queue_;
    //std::vector<std::pair<tbb::atomic<unsigned int>, Context*> > event_states;
};


#endif
