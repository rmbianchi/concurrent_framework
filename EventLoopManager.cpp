//
//  EventLoopManager.cpp
//  
//
//  Created by Benedikt Hegner on 4/12/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "EventLoopManager.h"

EventLoopManager::EventLoopManager(const std::vector<AlgoBase*>& algos, Whiteboard& wb, const unsigned int n_parallel) : 
algo_pool_(algos,std::vector<int>(algos.size(),1)),
scheduler_(wb, n_parallel, algo_pool_,algos),
in_flight_(0), processed_(0), current_event_(0)

{


}

void EventLoopManager::run(int events){ 
    
    scheduler_.run_parallel(events);
//    do {        
//        // check if a new event can and should be started
//        if (in_flight_ < max_concurrent_events_ && processed+in_flight < n) {
//            Context* context(0);
//            bool whiteboard_available = wb_.get_context(context);
//            if (whiteboard_available){
//                EventState* event_state = new EventState(size);
//                event_states.push_back(event_state);
//                event_state->context = context;
//                context->write(processed+in_flight, "event","event");
//                ++current_event;
//                ++in_flight;  
//            } else {
//                printf("no whiteboard available\n");
//            }
//        } 
//
//        
//        
//    } while (processed < n);
}

//void EventLoopManager::notify_event_done(int event_id){
//}
