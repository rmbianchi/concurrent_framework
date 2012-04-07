//
//  TaskSchedule.h
//  
//  Concurrent execution prototype based on tbb::task
//
//  Created by Benedikt Hegner on 4/6/12.
//  Copyright (c) 2012 __CERN__. All rights reserved.
//

#ifndef _TaskSchedule_h
#define _TaskSchedule_h

// include tbb
#include <task.h>

// forward declaration
// each event has its own task graph
class TaskGraph;


/**
 * Task class wrapping around AlgoBase.h
 */
class AlgoTask :: public tbb:task {};

/**
 * Node for a TaskGraph; the task graph does the book keeping of what can be run. 
 */
// Question: should we add the context to this? I think so!

class TaskGraphNode {
public:
    void run(notify_sucessors(););
    void receive_notification();
    void register_predecessor();
    void register_sucessor();
    
private:
    void notify_sucessors();
    void notify_queue();
    std::vector<AlgoTask*> sucessors;
    unsigned int m_notification_counter;
    TaskQueue* m_queue;
    const unsigned int m_identifier;
};

/**
 * Scheduling class based on bare tbb::task.
 */
class TaskSchedule {};


/**
 * Pool of Algos that are available for running 
 */



#endif
