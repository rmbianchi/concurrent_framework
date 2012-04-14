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
#include "tbb/concurrent_hash_map.h"
// include c++
#include <list>
#include <vector>
#include <map>
// include fwk
#include "ConcurrentTypes.h"
#include "Context.h"


enum ContextStatus {available, in_use, in_cleanup};

/**
 * the Whiteboard class features a concurrent_hash_map to store published products
 * from modules.
 * As opposed to operations done via Context this class is not thread safe.
 */
class Whiteboard {
public:
    Whiteboard( const char* name, const int number_of_slots);
    virtual ~Whiteboard();
    const char* get_name() {return name_;};
    // TODO: do a proper handling of object ownership 
    bool read(DataItem&, const std::string& label, const unsigned int slot_number) const;
    void write(const DataItem& item, const::std::string& label, const unsigned int slot_number); 
    void notify(const void* what = 0);
    void print_slot_content(const unsigned  int slot_number) const; 
    bool get_context(Context*&);
    void release_context(Context*& context);
    unsigned int register_dependency(const std::string& label); // registers a dependency and returns the index in the later used bit mask
private:
    const char* name_;
    const int number_of_slots_;
    unsigned int data_id_counter_;
    std::vector<StringDataMap*> slots_;
    typedef std::pair<Context*,ContextStatus> registry_type;
    std::vector<registry_type> contexts_; 
    std::map<std::string,unsigned int> data_id_map_;

};

#endif /* WHITEBOARD_H_ */
