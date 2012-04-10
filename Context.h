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
// include tbb
#include "tbb/spin_mutex.h"
#include "tbb/concurrent_hash_map.h"
// include fwk
#include "ConcurrentTypes.h"

// forward declaration
class Whiteboard;

class Context {
public:
    Context(const int number, Whiteboard& wb);
    virtual ~Context();
    bool read(DataItem&, const std::string& label) const;
    void write(const DataItem& item, const::std::string& algo_name, const::std::string& label); 
    void print_content() const;
    void set_finished(){m_finished=true;};
    const bool is_finished() const {return m_finished;};
    const unsigned int get_slotnumber() const {return m_slotnumber;};
    void reset();
private:
    const unsigned int m_slotnumber;
    bool m_finished;
    Whiteboard& wb; 
};

#endif /* CONTEXT_H_ */
