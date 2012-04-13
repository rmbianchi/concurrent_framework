//
//  EventLoopManager.h
//  
//
//  Created by Benedikt Hegner on 4/12/12.
//  Copyright (c) 2012 __CERN__. All rights reserved.
//

#ifndef _EventLoopManager_h
#define _EventLoopManager_h

// include tbb
#include "tbb/atomic.h"
// include fwk
#include "Algo.h"
#include "Scheduler.h"
#include "Whiteboard.h"

class Scheduler_;

class EventLoopManager{
    
public:
    EventLoopManager(const std::vector<AlgoBase*>& algos, Whiteboard& wb, const unsigned int n_parallel);
    void run(int events);
    void event_done();
    
private:
    AlgoPool algo_pool_;
    Scheduler scheduler_;
    tbb::atomic<unsigned int> in_flight_;
    tbb::atomic<unsigned int> processed_; 
};


#endif
