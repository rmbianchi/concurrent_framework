//
//  ExampleGraphs.h
//  CF4Hep
//
//  This file provides canonical test collections of algos
//
//  Created by Benedikt Hegner on 4/7/12.
//  Copyright (c) 2012 __CERN__. All rights reserved.
//

#ifndef CF4Hep_ExampleGraphs_h
#define CF4Hep_ExampleGraphs_h

// include c++
#include <vector>
// include fwk
#include "Algo.h"

// NOTE: it's a prototype, 
// for now the ownership of the algos is transferred to the caller.
std::vector<AlgoBase*> exampleChain1(){
    std::vector<AlgoBase*> algos;
	AlgoBase* algo0 = new ToyAlgo("algo0",1, 1); algo0->produces("hits");
    AlgoBase* algo1 = new ToyAlgo("algo1",2, 1); algo1->reads("hits"); algo1->produces("muons");
    AlgoBase* algo2 = new NonReentrantToyAlgo("algo2",3, 3); algo2->reads("hits"); algo2->produces("electrons");
	AlgoBase* algo3 = new ToyAlgo("algo3",4, 1); algo3->reads("muons"); algo3->reads("electrons"); algo3->produces("nobelprize");
    algos.push_back(algo0);
    algos.push_back(algo1);
    algos.push_back(algo2);
    algos.push_back(algo3);	
    return algos;
};

#endif
