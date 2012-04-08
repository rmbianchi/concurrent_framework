/**
 * Whiteboard.h
 *
 *  Created on: Mar 26, 2012
 *      Authors: rbianchi@cern.ch  (Riccardo-Maria BIANCHI)
 *               benedikt.hegner@cern.ch  (Benedikt HEGNER) 
 */

#ifndef WHITEBOARD_H_
#define WHITEBOARD_H_


// include TBB
#include "tbb/spin_mutex.h"
#include "tbb/concurrent_hash_map.h"
// include c++
#include <list>
#include <vector>
// include FWK specific headers
#include "ConcurrentTypes.h"
#include "Context.h"

// forward declarations
class Subscriber;

/**
 * the Whiteboard class features a concurrent_hash_map to store published products
 * from modules.
 *
 */
class Whiteboard {
public:
    // methods
    Whiteboard( const char* name, const int number_of_slots);
    virtual ~Whiteboard();
    const char* getName() {return my_name;};
    // I/O methods
    // TODO: do a proper handling of object ownership 
    bool read(DataItem&, const std::string& label, const int slot_number) const;
    void write(const DataItem& item, const::std::string& label, const int slot_number); 
    void notify(const void* what = 0);
    void print_slot_content(const int slot_number) const; 
    Context* getContext(const int i);

private:
    const char* my_name;
    const int number_of_slots;
    tbb::spin_mutex my_mutex;
    std::vector<StringDataMap*> m_slots;
    std::vector<Context*> m_contexts;
    //TODO: could make the graphs part of the slots as well

};

#endif /* WHITEBOARD_H_ */
