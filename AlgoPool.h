//
//  AlgoPool.h
//  
//
//  Created by Benedikt Hegner on 4/12/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef _AlgoPool_h
#define _AlgoPool_h

// include tbb
#include "tbb/concurrent_queue.h"
// include fwk
#include "Algo.h"


/**
 * The AlgoPool does the book keeping for all available Algo instances
 * TODO: It as well is the owner of the instances and responsible for their destruction
 */
class AlgoPool {
public:
    AlgoPool(const std::vector<AlgoBase*>& algos, const std::vector<int> max_instances);
    bool acquire(AlgoBase*& algo, const unsigned int algo_id);
    void release(AlgoBase*& algo, const unsigned int algo_id);
    
private:
    std::vector<tbb::concurrent_queue<AlgoBase*>*> available_algo_instances_;



};


#endif
