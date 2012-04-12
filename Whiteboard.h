/**
 * Whiteboard.h
 *
 *  Created on: Mar 26, 2012
 *      Authors: rbianchi@cern.ch  (Riccardo-Maria BIANCHI)
 *               benedikt.hegner@cern.ch  (Benedikt HEGNER) 
 */

#ifndef WHITEBOARD_H_
#define WHITEBOARD_H_


// include tbb
#include "tbb/spin_mutex.h"
#include "tbb/concurrent_hash_map.h"
// include c++
#include <list>
#include <vector>
#include <map>
// include fwk
#include "ConcurrentTypes.h"
#include "Context.h"

// forward declarations
class Subscriber;

/**
 * the Whiteboard class features a concurrent_hash_map to store published products
 * from modules.
 * As opposed to operations done via Context this class is not thread safe.
 */

enum ContextStatus {available, in_use, in_cleanup};

class Whiteboard {
public:
    // methods
    Whiteboard( const char* name, const int number_of_slots);
    virtual ~Whiteboard();
    const char* get_name() {return name_;};
    // I/O methods
    // TODO: do a proper handling of object ownership 
    bool read(DataItem&, const std::string& label, const unsigned int slot_number) const;
    void write(const DataItem& item, const::std::string& label, const unsigned int slot_number); 
    void notify(const void* what = 0);
    void print_slot_content(const unsigned  int slot_number) const; 
    bool get_context(Context*&);
    void release_context(Context*& context);
    unsigned int register_dependency(const std::string& label);

private:
    const char* name_;
    const int number_of_slots_;
    unsigned int data_id_counter_;
    tbb::spin_mutex my_mutex;
    std::vector<StringDataMap*> slots_;
    typedef std::pair<Context*,ContextStatus> registry_type;
    std::vector<registry_type> contexts_; 
    std::map<std::string,unsigned int> data_id_map_;

};

#endif /* WHITEBOARD_H_ */
