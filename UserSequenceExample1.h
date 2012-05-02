//
//  UserSequenceExample1.h
//  CF4Hep
//
//  Created by Riccardo Maria Bianchi on 5/2/12.
//  Copyright (c) 2012 CERN. All rights reserved.
//

#ifndef CF4Hep_UserSequenceExample1_h
#define CF4Hep_UserSequenceExample1_h

// include framework base class
#include "Sequence.h"


/**
 * A custom user class inheriting from the Sequence base class 
 * of the framework.
 */
class MyUserSequence : public Sequence {
    
public:
    MyUserSequence(const char *name, tbb::flow::graph &the_graph, std::vector<AlgoBase*> algorithms) : Sequence::Sequence(name, the_graph, algorithms) {};
};



#endif
