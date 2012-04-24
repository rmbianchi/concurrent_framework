//
//  EventLoopManager.h
//  
//
//  Created by Benedikt Hegner on 4/12/12.
//  Copyright (c) 2012 CERN. All rights reserved.
//

#ifndef _EventLoopManager_h
#define _EventLoopManager_h

// include tbb
#include "tbb/atomic.h"
// include fwk
#include "Algo.h"
#include "Scheduler.h"
#include "Whiteboard.h"

class EventLoopManager{
public:
    EventLoopManager(const unsigned int n_parallel);
    void initialise(int events, Scheduler* scheduler);
    void operator()();
    void finished_event();
private:
    Scheduler* scheduler_;
    tbb::atomic<unsigned int> in_flight_;
    tbb::atomic<unsigned int> processed_; 
    unsigned int max_concurrent_events_; 
    unsigned int events_;
};


class EventLoopManagerTask : public tbb::task {
    public:    
        EventLoopManagerTask(EventLoopManager* manager): manager_(manager){};    
        tbb::task* execute() { (*manager_)();return NULL; };
    private:    
        EventLoopManager* manager_;        
};

#endif
