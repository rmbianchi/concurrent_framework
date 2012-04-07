//
//  BitMapSchedule.h
//  CF4Hep
//
//  Created by Benedikt Hegner on 4/7/12.
//  Copyright (c) 2012 __CERN__. All rights reserved.
//

#ifndef CF4Hep_BitMapSchedule_h
#define CF4Hep_BitMapSchedule_h

#include "Algo.h"
#include "Scheduler.h"
#include "ConcurrentTypes.h"

class BitMapEvent {};

/**
 * the BitMapScheduler uses the BitMapEvent for book keeping of the event's state
 */
class BitMapScheduler: public Scheduler {
    
};

class BitMapEventFactory{

public:
    BitMapEventFactory(const std::vector<AlgoBase*>& algos){};
    const StringUIntMap get_algo_patterns( return m_algo_patterns);
    const unsigned int  get_event_template( return m_event_template);
    
private:
    unsigned int m_event_template; //how the finished event looks like
    StringUIntMap m_algo_patterns;


}:

BitMapEventFactory::BitMapEventFactory(const std::vector<AlgoBase*>& algos){

    for (unsigned int i = 0; i < algos.size(); ++i) {
        //do something
    }

};


#endif
