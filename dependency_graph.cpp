/**
 * dependency_graph.cpp
 *
 *  Created on: Mar 1, 2012
 *      Author: rbianchi@cern.ch  (Riccardo-Maria BIANCHI)
 */

// to print ROOT libs and paths: root-config --cflags --glibs


// include internals
#include "Algo.h"
#include "DataTypes.h"
// include c++
# include <iostream>
# include <cstdio>
#include <map>
// include tbb
# include "tbb/task_scheduler_init.h"
#include "tbb/flow_graph.h"

using namespace std;
using namespace tbb::flow;


tbb::spin_mutex my_mutex;

const int setup_time = 1;

const char *chainNames[] = { "Chain1", "Chain2", "Chain3", "Chain4", "Chain5",
"Chain6", "Chain7", "Chain8", "Chain9", "Chain10" };




//===========================
//		timing tools
//===========================
typedef unsigned long long timestamp_t;
static timestamp_t get_timestamp () {
  struct timeval now;
  gettimeofday (&now, NULL);
  return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}




//===========================
//		AlgoChain class
//===========================

class AlgoChain {
public:
	AlgoChain(const char *name, tbb::flow::graph &the_graph, std::vector<AlgoBase*> algorithms) :
		m_name(name), m_algorithms(algorithms), m_Nalgorithms(algorithms.size()), m_graph(&the_graph) {};
	virtual ~AlgoChain() {};
	void operator()();


private:
	struct body;
	const char* m_name;
	std::vector<AlgoBase*> m_algorithms;
	unsigned int m_Nalgorithms;
	std::map<int, function_node< continue_msg, continue_msg >* > m_created_nodes;



	// pointers to main graph elements
	tbb::flow::graph* m_graph;
	//broadcast_node< continue_msg >* m_start;

	// start nodes
	broadcast_node< continue_msg >* m_startNode;

	//friend class node_body;

	void setup();
	void make_my_node();
};

void AlgoChain::operator()() {
  setup();
  make_my_node();
  m_startNode->try_put( continue_msg() );
}

void AlgoChain::setup() {
	printf("\t%s setting up\n", m_name );
	sleep(setup_time);
	printf("\t%s done setup\n", m_name );
}


struct AlgoChain::body {
    std::string my_name;
    std::string my_parentName;
    body( const char *name, const char* parentName ) : my_name(name), my_parentName(parentName) {}
    void operator()( continue_msg ) const {
    	tbb::spin_mutex::scoped_lock lock(my_mutex);
        printf("\t\t%s: algo %s\n", my_parentName.c_str(), my_name.c_str());
    }
};

class node_body {
  AlgoBase* my_algorithm;
public:
  node_body( AlgoBase* p ) : my_algorithm(p) { }
  void operator()( continue_msg ) {
    my_algorithm->body();
  }
};

void AlgoChain::make_my_node() {
	// TODO Auto-generated constructor stub

	tbb::spin_mutex::scoped_lock lock;

	// connecting the subgraph to the main graph
	m_startNode = new broadcast_node< continue_msg >(*m_graph);

	// building nodes and connecting them with edges
	lock.acquire(my_mutex);
	printf("building nodes and edges...\n");
	lock.release();

	// loop over all algorithms scheduled for this job
	for (unsigned int aa = 0; aa < m_Nalgorithms; ++aa) {

		unsigned int algoN = aa;

		// make node if not created already
		if (!m_created_nodes.count(algoN)) {
			m_created_nodes[algoN] = new function_node< continue_msg, continue_msg >( *m_graph,
				tbb::flow::serial, node_body( m_algorithms[algoN] ));
		}

		// connecting the first-level nodes to the main graph
		std::string inputType = m_algorithms[algoN]->getDataTypeIN();
		std::cout << algoN << " " << m_algorithms[algoN]->getName() << " - input: " << inputType << std::endl;
		if (inputType.compare("DataTypeEvent") == 0 ) {
			make_edge( *m_startNode, *m_created_nodes[algoN] );
			lock.acquire(my_mutex);
			printf("\t connecting m_start --> %s (%u)\n", m_algorithms[algoN]->getName(), algoN);
			lock.release();
		}

		// output data type of the node
		std::string outDataType = m_algorithms[algoN]->getDataTypeOUT();

		// loop over all nodes to connect proper input with output data
		for (unsigned int node = 0; node < m_Nalgorithms; ++node ) {
			if (algoN != node) {
				std::string inDataType = m_algorithms[node]->getDataTypeIN();
				if (inDataType.compare(outDataType) == 0 ) {

					// create the node if not created yet
					if (!m_created_nodes.count(node)) {
						//m_created_nodes[node] = new function_node< continue_msg, continue_msg >( *m_graph, tbb::flow::serial, node_body( m_algorithms[node] )); // only one single copy of the body() allowed to be run concurrently
						m_created_nodes[node] = new function_node< continue_msg, continue_msg >( *m_graph, tbb::flow::unlimited, node_body( m_algorithms[node] ));
					}

					//connect nodes with edges
					make_edge(*m_created_nodes[algoN], *m_created_nodes[node] );
					lock.acquire(my_mutex);
					printf("\t\tconnecting %s --> %s - (%s --> %s) (%u-%u)\n", m_algorithms[algoN]->getName(), m_algorithms[node]->getName(), outDataType.c_str(), inDataType.c_str(), algoN, node);
					lock.release();
				}
			} // end if we are not considering the same algo in the two loops
		} // end inner loops on all algos to make connections
	} // end first loop over all algorithms in the chain, to build the nodes
	lock.acquire(my_mutex);
	printf("\t%s subgraph created\n", m_name);
	lock.release();

}




//===========================
//		Scheduler
//===========================
int schedule(std::vector< std::vector<AlgoBase*> > chainsVec) {


	unsigned int num_chains = chainsVec.size();
	printf("number of chains to schedule: %u\n", num_chains);

	// time it
	timestamp_t tstart = get_timestamp();

	// main graph
	printf("creating the main graph\n");
	graph g;

	// sub-graphs
	printf("creating and starting the AlgoChains graphs\n");
	std::vector< AlgoChain > chains;

	// loop over chains and runs the jobs
	for (unsigned int ch = 0; ch < num_chains; ++ch) {
		printf("instantiating Chain %u\n", ch);
		chains.push_back( AlgoChain( chainNames[ch], g, chainsVec[ch]) );
		g.run( chains[ch] );
	}


	tbb::spin_mutex::scoped_lock lock;

	// wait for all jobs completing
	lock.acquire(my_mutex);
	printf("waiting for all completing\n");
	lock.release();
	g.wait_for_all();

	// time it
	timestamp_t tstop = get_timestamp();
	timestamp_t totTime = tstop-tstart;
	printf("elapsed time: %llu \n", totTime);

	return totTime;
}



//===========================
//		MAIN
//===========================

int main(int argc, char *argv[]) {

	// default threads
	int num_threads = 4;


	// create a pool of algorithms
	printf("creating the pool of algos\n");
	Algo<DataTypeEvent, DataTypeA> algo1("algo1", "DataTypeEvent", "DataTypeA");
	Algo<DataTypeA, DataTypeE> algo2("algo2", "DataTypeA", "DataTypeE");
	Algo<DataTypeEvent, DataTypeB> algo3("algo3", "DataTypeEvent", "DataTypeB");
	Algo<DataTypeB, DataTypeC> algo4("algo4", "DataTypeB", "DataTypeC");
	Algo<DataTypeC, DataTypeE> algo5("algo5", "DataTypeC", "DataTypeE");
	Algo<DataTypeEvent, DataTypeB> algo6("algo6", "DataTypeEvent", "DataTypeB");
	Algo<DataTypeB, DataTypeE> algo7("algo7", "DataTypeB", "DataTypeE");

	/// a vector of algorithms for one job/chain
	//std::vector<AlgoBase*> algorithms;

	printf("assigning algorithms to jobs/chains\n");
	/// a vector of jobs/chains
	std::vector< std::vector<AlgoBase*> > chains;

	// gathering the algorithm for a first job/chain
	std::vector<AlgoBase*> chain1;
	chain1.push_back(&algo1);
	chain1.push_back(&algo2);
	// gathering the algorithm for a second job/chain
	std::vector<AlgoBase*> chain2;
	chain2.push_back(&algo1);
	chain2.push_back(&algo2);
	chain2.push_back(&algo3);
	chain2.push_back(&algo4);
	chain2.push_back(&algo5);
	chain2.push_back(&algo6);
	chain2.push_back(&algo7);

	// booking jobs/chains
	chains.push_back(chain1);
	chains.push_back(chain2);
	chains.push_back(chain2);
	chains.push_back(chain2);

	unsigned int num_chains = chains.size();


	// command-line parser
	if ( argc > 1 ) num_threads = atoi(argv[1]);

	if ( num_threads < 1 ) exit(1);

	// enabling a certain number of working threads
	printf("Init %i working threads\n", num_threads);
	tbb::task_scheduler_init init(num_threads); //apparently this value can be changed only once per application run


	bool test = false;
	if ( argc > 3 && atoi(argv[3]) == 1 ) test = true;

	if(test) {
			timestamp_t time = 0;
			int times = 0;
			for (int nn=0; nn<5; ++nn) {
				time += schedule(chains);
				++times;
			}
			printf("%i threads - %u chains - Time: %f\n\n\n", num_threads, num_chains, time/(double)times );
	}
	else {
		schedule(chains);
	}

    return 0;
}

