//
//  EventLoopManager.cpp
//  
//
//  Created by Benedikt Hegner on 4/12/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "EventLoopManager.h"
#include "tbb/compat/thread"

EventLoopManager::EventLoopManager(const std::vector<AlgoBase*>& algos, Whiteboard& wb, const unsigned int n_parallel) : 
algo_pool_(algos,std::vector<int>(algos.size(),1)),
scheduler_(wb, n_parallel, algo_pool_,algos, this),
in_flight_(), processed_()

{


}

void EventLoopManager::run(int events){ 
    scheduler_.run_parallel(events);
//    do {
//        if (in_flight_ < max_concurrent_events_ && processed+in_flight < n) {
//            scheduler_.start_event(processed+in_flight);
//            ++in_flight_;
//        }
///       std::this_thread::yield();
//    } while (processed_ < events);
}

void EventLoopManager::event_done(){
    ++processed_;
    --in_flight_;
}
