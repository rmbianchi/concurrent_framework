//
//  Skep.cpp
//  
//
//  Created by Benedikt Hegner on 4/14/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "tbb/tbb.h"

#include "LHCbGraph.h"
#include "Skep.h"


Skep::Skep(unsigned int events, unsigned int n_parallel_, unsigned int n_wb_slots) : 
events_(events), n_parallel_(n_parallel_), n_wb_slots_(n_wb_slots), 
event_loop_(n_parallel_), scheduler_(wb_), wb_("Central Whiteboard", n_wb_slots) {
    // inititalise the setup
    algos_ = lhcbChain();
    algo_pool_ = AlgoPool(algos_,std::vector<int>(algos_.size(),1));
    event_loop_.initialise(events, &scheduler_);
    scheduler_.initialise(&algo_pool_, &algos_, &event_loop_);    
    // now get things started
    tbb::task* e_task = new( tbb::task::allocate_root() ) EventLoopManagerTask(&event_loop_);
    tbb::task* s_task = new( tbb::task::allocate_root() ) SchedulerTask(&scheduler_);
    tbb::task::spawn(*s_task);
    tbb::task::spawn_root_and_wait(*e_task);
    //tbb::task_group tg;
    //tg.run(event_loop_);
    //tg.run(scheduler_);
    //tg.wait( );

}

Skep::~Skep(){
    for (unsigned int i = 0; i < algos_.size(); ++i) {
        delete algos_[i];
    }
}