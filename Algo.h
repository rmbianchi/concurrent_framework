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


enum AlgoStatus {not_run, successful, accept, reject};

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

    ToyAlgo(const char* name, int value, unsigned int time) : name_(name), inputs_(), outputs_(), time_(time), data_(value) 
    {printf("Instantiating %s\n", name_);};
    virtual ~ToyAlgo() {};

    // actual implementations of the virtual methods
    void body(Context *context) {
        unsigned int event(0);
        context->read(event, "event");
        //printf("Algo '%s' - begin - EVENT: %i\n", name_, event);
        sleep(time_);
        read(context);
        publish(context);
        //printf("Algo '%s' - end - EVENT: %i\n", name_, event);
    };
    const std::vector<std::string> get_inputs() const {return inputs_;};
    const std::vector<std::string> get_outputs() const {return outputs_;};
    const char* get_name() const {return name_;};

    void produces(const std::string& out) {outputs_.push_back(out);};
    void reads(const std::string& in) {inputs_.push_back(in);};
private:
    void publish(Context* context) {
       for (t_tags::const_iterator i=outputs_.begin(); i!=outputs_.end(); ++i) context->write(data_, name_, *i);
    };
    void read(Context* context) {
        for (t_tags::const_iterator i=inputs_.begin(); i!=inputs_.end(); ++i) context->read(data_, *i);
    }; 
protected:    
    typedef std::vector<std::string> t_tags;
    const char* name_;
    t_tags inputs_;
    t_tags outputs_;
    const unsigned int time_;
    DataItem data_;
};

/**
 * non re-entrant toy algorithm class; for now doing non-performant locking
 * could try to model it in tbb::flow:graph with a rejecting node
 * the serial specifier did not what I naively expected
 */
class NonReentrantToyAlgo : public ToyAlgo{
public:
    NonReentrantToyAlgo(const char* name, int value, unsigned int time) : ToyAlgo(name, value, time), counter_(0){};  
    void body(Context* context) {
        ++counter_;ToyAlgo::body(context); printf("Algo '%s' unsafe counter: %i\n", name_, counter_);
    };
private:
    int counter_;
    
};

#endif /* ALGO_H_ */
