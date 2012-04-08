/**
 * dependency_graph.cpp
 *
 *  Created on: Mar 1, 2012
 *      Author: rbianchi@cern.ch  (Riccardo-Maria BIANCHI)
 *              benedikt.hegner@cern.ch  (Benedikt HEGNER) 
 */

// include internals
#include "Algo.h"
#include "EventGraph.h"
#include "Whiteboard.h"
// include c++
#include <iostream>
#include <cstdio>
#include <map>


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
//		Scheduler
//===========================
int schedule(Whiteboard * wb, std::vector<AlgoBase*> chain) {

	// time it
	timestamp_t tstart = get_timestamp();

	// main graph
   tbb::flow::graph g;

	// loop over GraphSchedules and runs the jobs
    //EventGraph event_graph("a_name", g, chain);
	for (unsigned int ch = 0; ch < 2;++ch) {
	    Context* context = wb->getContext(ch); 
        context->write(ch, "event","eventid");
		printf("\nCreating graph of event %i:\n", ch);
        EventGraph event_graph("a_name", g, chain);
		g.run( EventGraphProxy( context, event_graph));
	}

	tbb::spin_mutex::scoped_lock lock;

	// wait for all jobs completing
	lock.acquire(my_mutex);
	printf("\nWaiting for all completing\n");
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
	// declaring a Whiteboard instance with a number of internal slots
	Whiteboard wb("Central Whiteboard", 3);
	// create a pool of toy algorithms
	printf("Creating the pool of algos:\n");
	ToyAlgo algo0("algo0",1, 1); algo0.produces("hits");
	ToyAlgo algo1("algo1",2, 1); algo1.reads("hits"); algo1.produces("muons");
	NonReentrantToyAlgo algo2("algo2",3, 3); algo2.reads("hits"); algo2.produces("electrons");
	ToyAlgo algo3("algo3",4, 1); algo3.reads("muons"); algo3.reads("electrons"); algo3.produces("nobelprize");
    // gathering the algorithm for a chain
	std::vector<AlgoBase*> chain;
	chain.push_back(&algo0);
	chain.push_back(&algo1);
	chain.push_back(&algo2);
	chain.push_back(&algo3);

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
				time += schedule(&wb, chain);
				++times;
			}
			printf("%i threads -  Time: %f\n\n\n", num_threads, time/(double)times );
	}
	else {
		schedule(&wb, chain);
	}

    wb.print_slot_content(0);
    wb.print_slot_content(1);

}

