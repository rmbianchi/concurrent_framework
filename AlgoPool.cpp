//
//  AlgoPool.cpp
//  
//
//  Created by Benedikt Hegner on 4/12/12.
//  Copyright (c) 2012 __CERN__. All rights reserved.
//
#include "AlgoPool.h"

AlgoPool::AlgoPool(const std::vector<AlgoBase*>& algos, const std::vector<int> max_instances){
    //TODO: use the max_instances structure!
    // Fill the data structure holding all available algorithm instances
    const unsigned int size = algos.size();
    available_algo_instances_.resize(size);
    for (unsigned int i = 0; i<size; ++i) {
        available_algo_instances_[i] = new tbb::concurrent_queue<AlgoBase*>();   
        available_algo_instances_[i]->push(algos[i]);
    }
}

bool AlgoPool::acquire(AlgoBase*& algo, const unsigned int algo_id){
    return available_algo_instances_[algo_id]->try_pop(algo); 
}

void AlgoPool::release(AlgoBase*& algo, const unsigned int algo_id){
    available_algo_instances_[algo_id]->push(algo);
}


