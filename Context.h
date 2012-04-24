/**
 * Context.h
 *
 *  Created on: Feb 8, 2012
 *      Author: benedikt.hegner@cern.ch  (Benedikt HEGNER) 
 */

#ifndef CONTEXT_H_
#define CONTEXT_H_

// include c++
#include <list>
#include <vector>
#include <bitset>
// include tbb
#include "tbb/spin_mutex.h"
#include "tbb/concurrent_hash_map.h"
// include fwk
#include "ConcurrentTypes.h"

// forward declaration
class Whiteboard;

typedef std::bitset<1000> context_state_type;

class Context {
public:
    Context(const int number, Whiteboard& wb);
    virtual ~Context();
    bool read(DataItem&, const std::string& label) const;
    void write(const DataItem& item, const::std::string& algo_name, const::std::string& label); 
    void print_content() const;
    void set_finished(){finished_=true;};
    const bool is_finished() const {return finished_;};
    const unsigned int get_slotnumber() const {return slotnumber_;};
    void reset();
    const context_state_type& get_state();
private:
    const unsigned int slotnumber_;
    bool finished_;
    Whiteboard& wb_; 
    context_state_type state_;
};

#endif /* CONTEXT_H_ */
