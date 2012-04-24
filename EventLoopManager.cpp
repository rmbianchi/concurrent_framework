//
//  EventLoopManager.cpp
//  
//
//  Created by Benedikt Hegner on 4/12/12.
//  Copyright (c) 2012 CERN. All rights reserved.
//

#include "EventLoopManager.h"
#include "tbb/compat/thread"

EventLoopManager::EventLoopManager(const unsigned int n_parallel) : 
in_flight_(), processed_(),
max_concurrent_events_(n_parallel), events_(0)
{
}

void EventLoopManager::initialise(int events, Scheduler* scheduler){
    events_ = events;
    scheduler_ = scheduler;
}

void EventLoopManager::finished_event(){
    ++processed_;
    --in_flight_;
}

void EventLoopManager::operator()(){
    do {
        if (in_flight_ < max_concurrent_events_ && processed_+in_flight_ < events_) {
            scheduler_->start_event(processed_+in_flight_);
            ++in_flight_;
        }
        std::this_thread::yield();
    } while (processed_ < events_);
    scheduler_->stop();
}