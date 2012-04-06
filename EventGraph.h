//
//  GraphSchedule.h
//  
//
//  Created by Benedikt Hegner on 4/5/12.
//  Copyright (c) 2012 __CERN__. All rights reserved.
//

#ifndef _GraphSchedule_h
#define _GraphSchedule_h

// include c++
#include <cstdio>
#include <map>
// include tbb
#include "tbb/task_scheduler_init.h"
#include "tbb/flow_graph.h"
#include "tbb/spin_mutex.h"
// include fwk
#include "Algo.h"
#include "DirtyTBBAdditions.h"

tbb::spin_mutex my_mutex;
const int setup_time = 1;

// for debugging: either use the special context node or the old function_node implementations
typedef dirty_new_node used_node;
//typedef tbb::flow::function_node< Context*, Context* > used_node;

class node_body {
    AlgoBase* my_algorithm;
public:
    node_body( AlgoBase* p ) : my_algorithm(p) { }
    Context* operator()( Context* context) {
        my_algorithm->body(context);
        return context; 
    }
};

//// EXPERIMENTAL BEGIN
//class node_body_after_join {
//    AlgoBase* my_algorithm;
//public:
//    node_body_after_join( AlgoBase* p ) : my_algorithm(p) { }
//    Context* operator()( std::tuple<Context*, Context*> context_tuple) {
//        my_algorithm->body(std::get<0>(context_tuple));
//        return std::get<0>(context_tuple); 
//    }
//};
//// EXPERIMENTAL END

/**
 * graph class to descripe the logical dependencies via a tbb::flow::graph
 */
class EventGraph {
public:
	EventGraph(const char *name, tbb::flow::graph &the_graph, std::vector<AlgoBase*> algorithms) :
    m_name(name), m_algorithms(algorithms), m_Nalgorithms(algorithms.size()),
    m_graph(&the_graph) {make_my_node();};
	virtual ~EventGraph() {};
	void operator()();
	const char* getName() const {return m_name;};
    tbb::flow::broadcast_node< Context* >* get_start_node(){return m_startNode;};
    
private:
	//struct body;
	const char* m_name;
	std::vector<AlgoBase*> m_algorithms;
	unsigned int m_Nalgorithms;
	std::map<int, used_node* > m_created_nodes;
        
	// pointer to main graph elements
	tbb::flow::graph* m_graph;
    
	// start nodes
    tbb::flow::broadcast_node< Context* >* m_startNode;    
    
	void make_my_node();
};


void EventGraph::make_my_node() {
    
	tbb::spin_mutex::scoped_lock lock;
    
	// connecting the subgraph to the main graph
	m_startNode = new tbb::flow::broadcast_node< Context* >(*m_graph);
    
	// building nodes and connecting them with edges
	// loop over all algorithms scheduled for this job
	for (unsigned int algoN = 0; algoN < m_Nalgorithms; ++algoN) {
		// make node if not created already
		if (!m_created_nodes.count(algoN)) {
            m_created_nodes[algoN] = new used_node( *m_graph,
                                                   tbb::flow::serial, 
                                                   node_body( m_algorithms[algoN] ));
		}
        
		// connecting the first-level nodes to the main graph
        std::vector<std::string> inputs = m_algorithms[algoN]->get_inputs();
        if (inputs.size() == 0 ) {
			make_edge( *m_startNode, *m_created_nodes[algoN] );
			lock.acquire(my_mutex);
			printf("-1: Connecting source with %s (%u)\n", m_algorithms[algoN]->getName(), algoN);
			lock.release();
		}
		printf(" %i: %s\n",algoN, m_algorithms[algoN]->getName());
        
		// outputs of the node
		std::vector<std::string> outputs = m_algorithms[algoN]->get_outputs();
        
        // loop over all outputs
        for (unsigned int out_counter = 0; out_counter < outputs.size(); ++out_counter ) {
		// loop over all nodes to connect proper input with output data
		for (unsigned int node = 0; node < m_Nalgorithms; ++node ) {
			if (algoN != node) {
				std::vector<std::string> inputs = m_algorithms[node]->get_inputs();
                for (unsigned int in_counter = 0; in_counter < inputs.size(); ++in_counter ) {
				if (inputs[in_counter].compare(outputs[out_counter]) == 0 ) {
                    
					// create the node if not created yet
					if (!m_created_nodes.count(node)) {
                        m_created_nodes[node] = new used_node( *m_graph, tbb::flow::serial, node_body( m_algorithms[node] ));
					}
                    
					//connect nodes with edges
					make_edge(*m_created_nodes[algoN], *m_created_nodes[node] );
					lock.acquire(my_mutex);
					printf("\tconnecting to %s (via '%s') (%u-%u)\n", m_algorithms[node]->getName(), inputs[in_counter].c_str(), algoN, node);
					lock.release();
				}
              } // end looping over all inputs
			} // end if we are not considering the same algo in the two loops
		} // end inner loops on all algos to make connections
      } // end loop on all outputs
	} // end first loop over all algorithms in the chain, to build the nodes
    
    
    //create a sink; 
    //TODO: 
    //  make it more elegant and remove the 'new's
    //  create a special sink class that calls back to the Whiteboard and declares the event as finished
    ToyAlgo* sink = new ToyAlgo("the sink",0, 0);sink->produces("output");
    used_node* sink_node = new used_node( *m_graph, tbb::flow::serial, node_body( sink ));
    
    for (unsigned int index = 0; index < m_Nalgorithms; ++index) {
        make_edge(*m_created_nodes[index],*sink_node);    
    }
    
    //TODO: check all nodes w/o successor and connect them to a sink node 
    
//    // EXPERIMENTAL BEGIN
//    // despite the join node, the after_join_node is being executed twice as often as needed
//    typedef tbb::flow::join_node<std::tuple<Context*, Context*>, tbb::flow::queueing>  join_node_type; 
//    join_node_type* j = new join_node_type(*m_graph);
//    m_created_nodes[3]->register_successor(tbb::flow::input_port<0>(*j));
//    m_created_nodes[3]->register_successor(tbb::flow::input_port<1>(*j));
//    ToyAlgo* foo = new ToyAlgo("blubb",2, 1);foo->produces("muons");
//    typedef tbb::flow::function_node<std::tuple<Context*, Context*> > function_node_type;
//    function_node_type* after_join_node = new function_node_type( *m_graph, tbb::flow::serial, node_body_after_join( foo ));
//    make_edge(*j,*after_join_node);
//	lock.acquire(my_mutex);
//	printf("\t%s subgraph created\n", m_name);
//	lock.release();
//    // EXPERIMENTAL END
    
}

/**
 * Proxy to the event graph for different contexts
 */
class EventGraphProxy {
public:
    EventGraphProxy(Context* context, EventGraph& graph) :
    m_context(context), m_startNode(graph.get_start_node())
    {};
    virtual ~EventGraphProxy() {};
    void operator()() {
        m_startNode->try_put( m_context );
    }
    
private:
    Context* m_context;
    tbb::flow::broadcast_node< Context* >* m_startNode;
    
};

#endif
