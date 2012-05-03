//
//  AlgoGraph.h
//  CF4Hep
//
//  Created by Riccardo Maria Bianchi on 5/3/12.
//  Copyright (c) 2012 CERN. All rights reserved.
//

#ifndef CF4Hep_AlgoGraph_h
#define CF4Hep_AlgoGraph_h


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
    const unsigned int get_identifier() const {return m_identifier;};
    
private:
    std::vector<TaskGraphNode*> m_sucessors;
    unsigned int m_notification_counter;
    AlgoBase* m_algo;
    TaskScheduler* m_scheduler;
    unsigned int m_identifier;
    unsigned int n_predecessors;
    unsigned int m_bitpattern;
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
    const std::vector<TaskGraphNode*>& get_all_nodes() const;
    // parts relevant for task based scheduling
    const bool finished() const {if (m_current_context==NULL) return false ; return m_current_context->is_finished();};
    const bool is_available() const {return m_available;};
    void reset(){m_available = true; m_current_context = NULL;};
    Context*& get_context(){return m_current_context;};
    
private:
    std::vector<AlgoBase*> m_algorithms;
    std::vector<TaskGraphNode*> m_nodes;
    void prepare_graph();
    AlgoBase* m_start_algo;
    TaskGraphNode* m_start_node;
    EndAlgo* m_stop_algo;
    TaskGraphNode* m_stop_node;
    // two state-full variables (w.r.t the context)
    bool m_available;
    Context* m_current_context;
    
};



#endif
