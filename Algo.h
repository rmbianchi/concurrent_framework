/**
 * Algo.h
 *
 *  Created on: Mar 8, 2012
 *      Authors: rbianchi@cern.ch  (Riccardo-Maria BIANCHI)
 *               benedikt.hegner@cern.ch  (Benedikt HEGNER)
 */

#ifndef ALGO_H_
#define ALGO_H_

// include c++
# include <iostream>
# include <cstdio>
// include internals
#include "Whiteboard.h"

#include "tbb/queuing_mutex.h"

/**
 * Algorithm virtual base class
 */
class AlgoBase {
public:
    virtual ~AlgoBase(){};
    virtual void body(Context* context) = 0;
    virtual const std::vector<std::string> get_inputs() const = 0;
    virtual const std::vector<std::string> get_outputs() const = 0;
    virtual const char* get_name() const = 0;
    virtual void produces(const std::string&) = 0;
    virtual void reads(const std::string&) = 0;
};

/**
 * Toy algorithm class
 */
class ToyAlgo : public AlgoBase {
public:

    ToyAlgo(const char* name, int value, unsigned int time) : m_name(name), inputs(), outputs(), time(time), data(value) 
    {printf("Instantiating %s\n", m_name);};
    virtual ~ToyAlgo() {};

    // actual implementations of the virtual methods
    void body(Context *context) {
        unsigned int event(0);
        context->read(event, "event");
        printf("Algo '%s' - begin - EVENT: %i\n", m_name, event);
        sleep(time);
        read(context);
        publish(context);
        printf("Algo '%s' - end - EVENT: %i\n", m_name, event);
    };
    const std::vector<std::string> get_inputs() const {return inputs;};
    const std::vector<std::string> get_outputs() const {return outputs;};
    const char* get_name() const {return m_name;};

    void produces(const std::string& out) {outputs.push_back(out);};
    void reads(const std::string& in) {inputs.push_back(in);};
private:
    void publish(Context* context) {
       for (t_tags::const_iterator i=outputs.begin(); i!=outputs.end(); ++i) context->write(data, m_name, *i);
    };
    void read(Context* context) {
        for (t_tags::const_iterator i=inputs.begin(); i!=inputs.end(); ++i) context->read(data, *i);
    }; 
protected:    
    typedef std::vector<std::string> t_tags;
    const char* m_name;
    t_tags inputs;
    t_tags outputs;
    const unsigned int time;
    DataItem data;
};

/**
 * non re-entrant toy algorithm class; for now doing non-performant locking
 * could try to model it in tbb::flow:graph with a rejecting node
 * the serial specifier did not what I naively expected
 */
class NonReentrantToyAlgo : public ToyAlgo{
public:
    NonReentrantToyAlgo(const char* name, int value, unsigned int time) : ToyAlgo(name, value, time), m_counter(0){};  
    void body(Context* context) {
        tbb::queuing_mutex::scoped_lock lock;
        lock.acquire(my_mutex);
        ++m_counter;ToyAlgo::body(context); printf("Algo '%s' unsafe counter: %i\n", m_name, m_counter);
        lock.release();
    };
private:
    int m_counter;
    tbb::queuing_mutex my_mutex;
    
};

class EndAlgo : public AlgoBase{
public:
    EndAlgo(const char* name) : m_name(name) {};  
    void body(Context* context) {
        tbb::queuing_mutex::scoped_lock lock;
        lock.acquire(my_mutex);
        context->set_finished();
        lock.release();
    };
    const std::vector<std::string> get_inputs() const {return std::vector<std::string>();};
    const std::vector<std::string> get_outputs() const {return std::vector<std::string>();};
    const char* get_name() const {return m_name;};
    void produces(const std::string&){};
    void reads(const std::string&){};

    
private:
    const char* m_name;
    tbb::queuing_mutex my_mutex;
};

#endif /* ALGO_H_ */
