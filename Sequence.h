//
//  Sequence.h
//  CF4Hep
//
//  Created by Riccardo Maria Bianchi on 4/30/12.
//  Copyright (c) 2012 CERN. All rights reserved.
//

#ifndef CF4Hep_Sequence_h
#define CF4Hep_Sequence_h

// include fwk
#include "AlgoPool.h"
#include "Scheduler.h"
// include c++
#include <vector>
// include tbb
#include "tbb/flow_graph.h"
//#include "tbb/spin_mutex.h"




const int setup_time = 1;




//class Sequence : public Subscriber {
class Sequence {
    
    
public:
    Sequence(const char *name, tbb::flow::graph &the_graph, std::vector<AlgoBase*> algorithms) :
    m_name_(name), m_algorithms_(algorithms), m_Nalgorithms_(algorithms.size()), m_graph(&the_graph) 
    {
        this->make_algo_map(m_algorithms_);
    };
    virtual ~Sequence() {};
    inline void operator()() {
        setup();
        make_my_sequence();
        m_startNode->try_put( tbb::flow::continue_msg() );
    };
    const char* getName() {return m_name_;};
    //void update(Whiteboard* who, const void* what = 0);
    
    
private:
    //struct body;
    const char* m_name_;
    std::vector<AlgoBase*> m_algorithms_;
    std::map<std::string, AlgoBase*> algo_map_;
    unsigned int m_Nalgorithms_;
    std::map<int, tbb::flow::function_node< tbb::flow::continue_msg, tbb::flow::continue_msg >* > m_created_nodes_;
    
    // pointer to main graph elements
    tbb::flow::graph* m_graph;
    
    // start nodes
    tbb::flow::broadcast_node< tbb::flow::continue_msg >* m_startNode;
    
    //*** methods ***
    void setup(); /// Custom setup of the Sequence
    void make_my_sequence(); ///Create the custom Sequence of algorithms
    void make_algo_map(std::vector<AlgoBase*> algos) {
        algo_map_.clear();
        for (unsigned int nn=0; nn < algos.size(); ++nn) {
            algo_map_[ algos[nn]->get_name()] = algos[nn];
        }
    }; /// create the map of <algoName,algoPointer>
    
};







//void Sequence::update(Whiteboard* who, const void* what)
//{
//    printf("\t%s - update() - Whiteboard %s notified the publication of product %s.\n", m_name, who->getName(), (char*)what );
//}



// this is called to start a task for the algo instance
class node_body {
    AlgoBase* my_algorithm;
public:
    node_body( AlgoBase* p ) : my_algorithm(p) { }
    void operator()( tbb::flow::continue_msg ) {
        //        my_algorithm->body();
    }
};








#endif
