//
//  Skep.h
//  
//
//  Created by Benedikt Hegner on 4/14/12.
//  Copyright (c) 2012 CERN. All rights reserved.
//

#ifndef _Skep_h
#define _Skep_h

// include fwk
#include "AlgoPool.h"
#include "EventLoopManager.h"
#include "Scheduler.h"
#include "Whiteboard.h"


class Skep {
public:
    Skep(unsigned int events, unsigned int n_parallel_, unsigned int n_wb_slots);
    virtual ~Skep();
private:
    const unsigned int events_;
    const unsigned int n_parallel_;
    const unsigned int n_wb_slots_;    
    EventLoopManager event_loop_;
    Scheduler scheduler_;
    Whiteboard wb_;
    std::vector<AlgoBase*> algos_;
    AlgoPool algo_pool_;
    
};


#endif
