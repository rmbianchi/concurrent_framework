//
//  EventLoopManager.cpp
//  
//
//  Created by Benedikt Hegner on 4/12/12.
//  Copyright (c) 2012 CERN. All rights reserved.
//

#include "EventLoopManager.h"
#include "tbb/compat/thread"

EventLoopManager::EventLoopManager(const std::vector<AlgoBase*>& algos, Whiteboard& wb, const unsigned int n_parallel) : 
algo_pool_(algos,std::vector<int>(algos.size(),1)), 
scheduler_(wb, n_parallel, algo_pool_,algos, this), 
in_flight_(), processed_(),
max_concurrent_events_(n_parallel), events_(0)
{
}

void EventLoopManager::start(int events){
    events_ = events;
    tbb::task* m = new( tbb::task::allocate_root() ) EventLoopManagerTask(this);
    scheduler_.initialise();
    tbb::task::spawn_root_and_wait(*m);
}

void EventLoopManager::finished_event(){
    ++processed_;
    --in_flight_;
}

void EventLoopManager::run(){
    printf("EventLoopManager::run\n");
    tbb::task* s_task = new( tbb::task::allocate_root() ) SchedulerTask(&scheduler_);
    //tbb::task::enqueue( *s_task); 
    tbb::task::spawn(*s_task);
    do {
        if (in_flight_ < max_concurrent_events_ && processed_+in_flight_ < events_) {
            scheduler_.start_event(processed_+in_flight_);
            ++in_flight_;
        }
        std::this_thread::yield();
    } while (processed_ < events_);
    sleep(1);
    scheduler_.stop();
    printf("EventLoopManager::run finished\n");

}