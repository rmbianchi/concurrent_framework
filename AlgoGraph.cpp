//
//  AlgoGraph.cpp
//  CF4Hep
//
//  Created by Riccardo Maria Bianchi on 5/3/12.
//  Copyright (c) 2012 CERN. All rights reserved.
//

#include "AlgoGraph.h"
#include <assert.h>
#include <iostream>

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


