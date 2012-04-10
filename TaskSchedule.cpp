//
//  TaskSchedule.cpp
//  
//
//  Created by Benedikt Hegner on 4/8/12.
//  Copyright (c) 2012 __CERN__. All rights reserved.
//

#include "TaskSchedule.h"
#include <assert.h>

tbb::task* ConcreteTask::execute(){
    m_task->first->execute(m_task->second, m_algo_instance);
    delete m_task;
    return NULL;
}


//===========================
//		TaskGraphNode
//===========================
TaskGraphNode::TaskGraphNode(AlgoBase* algo, const unsigned int index):
m_notification_counter(0), m_algo(algo), m_identifier(index), n_predecessors(0), m_bitpattern(0){};

void TaskGraphNode::register_predecessor(TaskGraphNode* node){
    ++n_predecessors;
    node->register_sucessor(this);
}


void TaskGraphNode::register_sucessor(TaskGraphNode* node){
    m_sucessors.push_back(node);
}


void TaskGraphNode::set_scheduler(TaskScheduler* scheduler){
    m_scheduler = scheduler; 
}
  

void TaskGraphNode::run_sequentially(Context* context){
    if (n_predecessors>0){++m_notification_counter;}; // cover the special case of start node
    if (m_notification_counter==n_predecessors){
        m_algo->body(context);
        for (unsigned int i=0; i < m_sucessors.size(); ++i) {
            m_sucessors[i]->run_sequentially(context);
        }
        m_notification_counter = 0;
    }
}


// For parallel execution. It puts a TaskItem into the queue
// TODO: check the counter for thread safety 
void TaskGraphNode::run_parallel(Context* context){
    if (n_predecessors>0){++m_notification_counter;}; // cover the special case of start node
    if (m_notification_counter==n_predecessors){
        m_scheduler->add_to_waiting_queue(new TaskItem(this,context));
        m_notification_counter = 0;
    }
}


void TaskGraphNode::notify_sucessors(Context* context){
    for (unsigned int i=0; i < m_sucessors.size(); ++i){
        m_sucessors[i]->run_parallel(context);
    }   
}


void TaskGraphNode::execute(Context* context, AlgoBase* algo_instance){
    algo_instance->body(context);
    notify_sucessors(context);
    m_scheduler->add_to_done_queue(new TaskItem(this,context));
}


//===========================
//		AlgoGraph
//===========================
AlgoGraph::AlgoGraph(std::vector<AlgoBase*> algorithms): m_algorithms(algorithms), m_available(true), m_current_context(0){
    prepare_graph();
}


AlgoGraph::~AlgoGraph(){
    for (unsigned int i=0, max=m_nodes.size(); i<max; ++i) {
        delete m_nodes[i];
    }   
}


const std::vector<TaskGraphNode*>& AlgoGraph::get_all_nodes() const {
    return m_nodes;
}

void AlgoGraph::run_sequentially(Context* context){
    m_current_context = context;
    m_start_node->run_sequentially(context);
}


void AlgoGraph::run_parallel(Context* context){
    m_current_context = context;
    m_available = false;
    m_start_node->run_parallel(context);
}


void AlgoGraph::prepare_graph(){
    
    //NOTE: for now not the fastest, but the most straight forward algorihm    
    m_start_algo = new ToyAlgo("event",0,0);
    m_start_node = new TaskGraphNode(m_start_algo,0);
    m_nodes.push_back(m_start_node);
    // create the TaskGraphNodes and create a map of output:node
    std::map<std::string,TaskGraphNode*> tmp_registry;
    for (unsigned int i = 0, nOfAlgos = m_algorithms.size(); i < nOfAlgos; ++i) {
        AlgoBase* algo = m_algorithms[i];
        
        TaskGraphNode* node = new TaskGraphNode(algo,i+1);  //i+1 since we added a start node
        m_nodes.push_back(node); 
        const std::vector<std::string> outputs = algo->get_outputs();
        for (unsigned int j = 0, n_outputs = outputs.size(); j < n_outputs; ++j){
            tmp_registry[outputs[j]] = node;
        }
    }
    // connect the nodes with each other; omit the start node
    // TODO: *not* safe against one algo reading more than one output from the same predecessor
    // I guess this we have to make sure in the TaskGraphNode
    for(unsigned int i = 1; i < m_nodes.size(); ++i){
        const AlgoBase* algo = m_nodes[i]->get_algo();
        printf(" %i: %s\n",i,algo->get_name());
        std::vector<std::string> inputs = algo->get_inputs();
        unsigned int n_of_inputs = inputs.size();
        if (n_of_inputs == 0) {m_nodes[i]->register_predecessor(m_start_node);}
        else {
            for(unsigned int j = 0; j < inputs.size(); ++j){
                std::string input = inputs[j];
                TaskGraphNode* source = tmp_registry[input];
                m_nodes[i]->register_predecessor(source);
                printf("\tconnecting to %s (via '%s')\n", source->get_algo()->get_name(), input.c_str());
            }
        }
    } // end of connecting the nodes
    m_stop_algo = new EndAlgo("*END*");
    m_stop_node = new TaskGraphNode(m_stop_algo,m_nodes.size());
    for(unsigned int i = 1; i < m_nodes.size(); ++i){
        if(m_nodes[i]->n_of_sucessors() == 0){
            m_stop_node->register_predecessor(m_nodes[i]);
        }     
    }
    m_nodes.push_back(m_stop_node);      
}    


//===========================
//		TaskScheduler
//===========================
TaskScheduler::TaskScheduler(std::vector<AlgoBase*> algos, Whiteboard* wb, unsigned int max_concurrent_events): m_max_concurrent_events(max_concurrent_events), m_algos(0), wb_(wb) {    
    assert(max_concurrent_events>0); // we want to have at least one graph
    for (unsigned int graph_index = 0; graph_index < max_concurrent_events; ++graph_index) {
        m_graphs.push_back(new AlgoGraph(algos));
        // announce itself to all GraphNodes
        const std::vector<TaskGraphNode*>& nodes = m_graphs[graph_index]->get_all_nodes();
        for (unsigned int i = 0, max = nodes.size(); i<max; ++i) {
            nodes[i]->set_scheduler(this);      
        }
    }
    
    //For now assume we have per algo one available instance
    //TODO: make this configurable; requires proper copy constructor of algos
    const unsigned int size = m_graphs[0]->get_all_nodes().size();
    available_algo_instances.resize(size);
    m_algos.resize(size);
    const std::vector<TaskGraphNode*>& nodes = m_graphs[0]->get_all_nodes();
    for (unsigned int i = 0, max = nodes.size(); i<max; ++i) {
        available_algo_instances[nodes[i]->get_identifier()] = new tbb::concurrent_queue<AlgoBase*>();   
        available_algo_instances[nodes[i]->get_identifier()]->push(nodes[i]->get_algo());
        m_algos[nodes[i]->get_identifier()] = nodes[i]->get_algo();
    }  
}


void TaskScheduler::add_to_waiting_queue(TaskItem* task_item) {
    m_waiting_queue.push(task_item);
}


void TaskScheduler::add_to_done_queue(TaskItem* task_item) {
    m_done_queue.push(task_item);
}


//TODO: make sure there are enough contexts available
void TaskScheduler::run_parallel(int n){
    printf("++++++++++++++++++++++++++++\n");
    printf(" Using scheduler flavour #1\n");
    printf("++++++++++++++++++++++++++++\n");
    unsigned int in_flight(0), processed(0);
    unsigned int current_event(0);
    
    do {
        // if possible start processing of a new event
        if (in_flight < m_max_concurrent_events && processed+in_flight < n) {
            AlgoGraph* available_graph(0);
            for (unsigned int i=0, max = m_graphs.size() ; i<max; ++i) {
                if (m_graphs[i]->is_available()) {available_graph = m_graphs[i];}
            }
            Context* context(0);
            wb_->get_context(context);
            context->write(processed+in_flight, "event","event");
            ++current_event;
            available_graph->run_parallel(context); 
            available_graph = 0;
            ++in_flight;
        }        
        
        TaskItem* result(0);  
        bool queue_full(false);
        bool algo_free(false);

        // put back in the waiting queue what was not successful in the previous looping
        do {
            queue_full = m_checked_queue.try_pop(result);
            if (queue_full){
                m_waiting_queue.push(result);
            }
        } while (queue_full);
        
        // put into the tbb queue whatever is possible
        queue_full = false;
        do {
            AlgoBase* algo_instance(0);//algo_instance = NULL;
            queue_full = m_waiting_queue.try_pop(result);
            if (queue_full) {
                algo_free = available_algo_instances[result->first->get_identifier()]->try_pop(algo_instance);
                if (algo_free) { 
                    tbb::task* t = new( tbb::task::allocate_root() ) ConcreteTask(result, algo_instance);
                    tbb::task::enqueue( *t);
                } else {
                    m_checked_queue.push(result);                }
            }
        } while (queue_full);
        
        // check for finished tasks and put the used algo instances back into the list of available ones
        queue_full = false;
        do {
            queue_full = m_done_queue.try_pop(result);
            if (queue_full) {
                available_algo_instances[result->first->get_identifier()]->push(result->first->get_algo());
            }
        } while (queue_full); 
                   
        // check for finished events
        for (unsigned int i=0, max = m_graphs.size() ; i<max; ++i) {
            if (m_graphs[i]->finished()) {++processed; --in_flight; m_graphs[i]->reset(); printf("Finished event\n"); }
        }        
    } while (processed < n);
    
}


void TaskScheduler::run_sequentially(int n){
    printf("++++++++++++++++++++++++++++\n");
    printf(" Sequential scheduling\n");
    printf("++++++++++++++++++++++++++++\n");
    for (unsigned int i = 0; i < n;++i) {
	    Context* context(0);
        wb_->get_context(context); 
        context->write(i, "event","event");
		m_graphs[0]->run_sequentially(context); 
	}
}
