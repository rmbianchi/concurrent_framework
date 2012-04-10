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
//include fwk
#include "Algo.h"
#include "Context.h"

/**
 * The AlgoTaskId the item used for call back once tbb finished the AlgoTask
 */
class AlgoTaskId {
public:
    AlgoTaskId(AlgoBase* algo, unsigned int algo_id, unsigned int event_id, Context* context): algo(algo),algo_id(algo_id), event_id(event_id), context(context) {};
    AlgoBase* algo;
    unsigned int algo_id;
    unsigned int event_id;
    Context* context;
};

/**
 * The tbb::task implementation that gets passed to tbb::task::enqueue
 * as opposed to AlgoTaskId it can be disposed by tbb (as per design)
 * it specifically gets the done_queue for talk back
 */
class AlgoTask : public tbb::task {
public:
    AlgoTask(AlgoTaskId* task, tbb::concurrent_queue<AlgoTaskId*>* done_queue): m_task(task), m_done_queue(done_queue){};    
    tbb::task* execute();
    AlgoTaskId* m_task;
    tbb::concurrent_queue<AlgoTaskId*>* m_done_queue;
};


class Scheduler {
public:
    Scheduler(const std::vector<AlgoBase*>& algorithms, Whiteboard& wb, unsigned int max_concurrent_events);
    void run_parallel(int n);
private:
    std::vector<unsigned int> compute_dependencies();
    std::vector<AlgoBase*> m_algos;
    Whiteboard& m_wb;
    unsigned int m_max_concurrent_events;
    std::vector<tbb::concurrent_queue<AlgoBase*>*> m_available_algo_instances;
    tbb::concurrent_queue<AlgoTaskId*> m_bits_done_queue;
};


#endif
