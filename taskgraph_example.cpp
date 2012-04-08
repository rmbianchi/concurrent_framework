//
//  taskgraph_example.cpp
//  CF4Hep
//
//  Created by Benedikt Hegner on 4/7/12.
//  Copyright (c) 2012 __CERN__. All rights reserved.
//

// include c++
#include <iostream>
// include tbb
#include "tbb/flow_graph.h"
#include "tbb/task_scheduler_init.h"
// include fwk
#include "Algo.h"
#include "ExampleChains.h"
#include "Helpers.h"
#include "Whiteboard.h"
#include "TaskSchedule.h"


tbb::spin_mutex my_mutex;

//===========================
//		Scheduler
//===========================
int schedule(Whiteboard * wb, std::vector<AlgoBase*> chain) {
    
	// time it
	timestamp_t tstart = get_timestamp();
    
	// set up the scheduler
    TaskScheduler scheduler(chain, wb, 3);
    scheduler.run_parallel(1);

	tbb::spin_mutex::scoped_lock lock;
    
	// wait for all jobs completing
	lock.acquire(my_mutex);
	printf("\nWaiting for all completing\n");
	lock.release();
    
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
	std::vector<AlgoBase*> chain = exampleChain2();
    
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

    // do a final cleanup
    for (unsigned int i = 0; i < chain.size(); ++i) {
        delete chain[i];
    }
}
