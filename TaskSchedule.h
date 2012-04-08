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

// include c++
#include <map>
#include <vector>
#include <string>
// include tbb
#include "tbb/concurrent_queue.h"
#include "tbb/concurrent_vector.h"
#include "tbb/task.h"
//include fwk
#include "Algo.h"
#include "Context.h"

// fwd declaration
class TaskScheduler;
class TaskGraphNode;

// the TaskItem is a combination of task+context
typedef std::pair<TaskGraphNode*, Context* > TaskItem;

/**
 * The tbb::task implementation that gets passed to tbb::task::enqueue
 * as opposed to TaskItem it can be disposed by tbb (as per design)
 * it specifically gets the instance of the algo to use
 */
class ConcreteTask : public tbb::task {
public:
    ConcreteTask(TaskItem* task, AlgoBase* instance): m_task(task), m_algo_instance(instance) {};    
    tbb::task* execute();
    TaskItem* m_task;
    AlgoBase* m_algo_instance;
};


/**
 * Node for a TaskGraph; the task graph does the book keeping of what can be run. 
 */
class TaskGraphNode {
public:
    TaskGraphNode(AlgoBase* algo, const unsigned int index);
    void run_sequentially(Context* context);
    void register_sucessor(TaskGraphNode* node);
    void register_predecessor(TaskGraphNode* node);
    AlgoBase* get_algo() const {return m_algo;};
    void set_algo(AlgoBase* algo) {m_algo = algo;};
    
    // parts relevant for task based scheduling
    void run_parallel(Context* context);
    void set_scheduler(TaskScheduler* scheduler);
    void notify_sucessors(Context* context);
    void execute(Context* context, AlgoBase* algo_instance);
    unsigned int n_of_sucessors(){return m_sucessors.size();};
    
private:
    void notify_sucessors(); // maybe we want to make the context part of this!
    //void notify_queue();
    std::vector<TaskGraphNode*> m_sucessors;
    unsigned int m_notification_counter;
    AlgoBase* m_algo;
    TaskScheduler* m_scheduler;
    unsigned int m_identifier;
    unsigned int n_predecessors;
};


/**
 * The graph containing all TaskGraphNodes. Once run, it forwards all smart action to the nodes 
 */
class AlgoGraph{
public:
    AlgoGraph(std::vector<AlgoBase*> algorithms);
    virtual ~AlgoGraph();
    void run_sequentially(Context*);
    void run_parallel(Context*);
    void get_bit_pattern(){};
    const std::vector<TaskGraphNode*>& get_all_nodes();
    const bool finished() const {return m_stop_algo->was_run();};
    const bool is_available() const {return m_available;};
    void reset(){m_available = true;};
    
private:
    bool m_available;
    std::vector<AlgoBase*> m_algorithms;
    std::vector<TaskGraphNode*> m_nodes;
    void prepare_graph();
    AlgoBase* m_start_algo;
    TaskGraphNode* m_start_node;
    EndAlgo* m_stop_algo;
    TaskGraphNode* m_stop_node;
};



/**
 * The TaskScheduler pushes tbb:tasks to the tbb internals, based on two inputs:
 *    1) the TaskNodes that could be run
 *    2) the available AlgoInstances 
 * Once the task finshed, the TaskScheduler frees the resources
 */
class TaskScheduler {
public:
    TaskScheduler(std::vector<AlgoBase*> algos, Whiteboard* wb, unsigned int max_concurrent_events);
    void add_to_waiting_queue(TaskItem* graph_node);
    void add_to_done_queue(TaskItem* graph_node);
    void print_queue(); 
    void run_parallel(int n); // run n events
    void run_sequentially(int n); // run n events
    
private:
    unsigned int m_max_concurrent_events;
    std::vector<AlgoBase*> m_algos;
    std::vector<AlgoGraph*> m_graphs;
    Whiteboard* m_wb;
    std::vector<tbb::concurrent_vector<AlgoBase*> > available_algo_instances;
    tbb::concurrent_queue<TaskItem*> m_waiting_queue;
    tbb::concurrent_queue<TaskItem*> m_done_queue;
              
};

#endif
