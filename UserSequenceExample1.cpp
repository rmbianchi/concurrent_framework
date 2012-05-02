//
//  Sequence.cpp
//  CF4Hep
//
//  Created by Riccardo Maria Bianchi on 5/1/12.
//  Copyright (c) 2012 CERN. All rights reserved.
//


// include user header
#include "UserSequenceExample1.h"

// include c++
#include <iostream>



tbb::spin_mutex m_mutex;






/**
 * Custom user implementation of the setup of the Sequence
 */
void Sequence::setup() {
    printf("\t%s setting up\n", m_name_ );
    sleep(setup_time);
    //printf("\t\t%s subscribe() to the Whiteboard's notifications\n", m_name );
    //m_wb->subscribe(this);
    printf("\t%s done setup\n", m_name_ );
}




/**
 * User implementation of custom Sequence of algorithms
 */
void Sequence::make_my_sequence() {
    
    tbb::spin_mutex::scoped_lock lock;
    
    // connecting the subgraph to the main graph
    m_startNode = new tbb::flow::broadcast_node< tbb::flow::continue_msg >(*m_graph);
    
    // building nodes and connecting them with edges
    lock.acquire(m_mutex);
    printf("building nodes and edges...\n");
    lock.release();
    
    
    /* algo_map_ contains the pointers to all the algorithms 
     * which can be used for the custom chain.
     * Here the user can build its own Sequence connecting the algoritms.
     */
    
    std::cout << " algo: " << algo_map_["algo0"]->get_name() << std::endl;
    
    
    
    
    
    
}





