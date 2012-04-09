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
    AlgoBase* algo0 = new ToyAlgo("algo0",1, 0); algo0->produces("hits");
    AlgoBase* algo1 = new ToyAlgo("algo1",2, 0); algo1->reads("hits"); algo1->produces("muons");
    AlgoBase* algo2 = new NonReentrantToyAlgo("algo2",3, 3); algo2->reads("hits"); algo2->produces("electrons");
    AlgoBase* algo3 = new ToyAlgo("algo3",4, 0); algo3->reads("muons"); algo3->reads("electrons"); algo3->produces("nobelprize");
    algos.push_back(algo0);
    algos.push_back(algo1);
    algos.push_back(algo2);
    algos.push_back(algo3);	
    return algos;
};

std::vector<AlgoBase*> exampleChain2(){
    std::vector<AlgoBase*> algos;
    AlgoBase* algo1 = new ToyAlgo("algo1",1, 1); algo1->produces("DataTypeA");
    AlgoBase* algo2 = new ToyAlgo("algo2",2, 1); algo2->reads("DataTypeA"); algo2->produces("DataTypeE");
    AlgoBase* algo3 = new ToyAlgo("algo3",3, 3); algo3->produces("DataTypeB");
    AlgoBase* algo4 = new ToyAlgo("algo4",4, 1); algo4->reads("DataTypeB");algo4->produces("DataTypeC");
    AlgoBase* algo5 = new ToyAlgo("algo5",4, 1); algo5->reads("DataTypeC");algo5->produces("DataTypeF");
    AlgoBase* algo6 = new ToyAlgo("algo6",4, 1); algo6->produces("DataTypeD");
    AlgoBase* algo7 = new ToyAlgo("algo7",4, 1); algo7->reads("DataTypeB");
    algos.push_back(algo1);
    algos.push_back(algo2);
    algos.push_back(algo3);
    algos.push_back(algo4);	
    algos.push_back(algo5);
    algos.push_back(algo6);
    algos.push_back(algo7);	
    return algos;
};

#endif
