/**
 * Whiteboard.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: rbianchi@cern.ch  (Riccardo-Maria BIANCHI)
 *              benedikt.hegner@cern.ch  (Benedikt HEGNER)
 */

// include internals
#include "Whiteboard.h"


Whiteboard::Whiteboard(const char* name, const int i) : name_(name), number_of_slots_(i), data_id_counter_(0) {
    printf("Init whiteboard with %i slots.\n",i);
    for (int i=0; i< number_of_slots_; ++i){
        contexts_.push_back(registry_type(new Context(i,*this), ContextStatus()));
        slots_.push_back(new StringDataMap());
    }
}

Whiteboard::~Whiteboard() {
    for (int i=0; i< number_of_slots_; ++i){
        delete contexts_[i].first;
        delete slots_[i];
    }
}

void Whiteboard::write ( const DataItem &item, const std::string &key, const unsigned int slot_number) {

    // use accessor as a cursor in the concurrent data structure
    // releases lock at destruction of accessor 
    StringDataMap::accessor a;
    StringDataMap* slot = slots_[slot_number];
    if (slot->count(key)>0) { printf("WARNING: %s already in whiteboard\n",key.c_str()) ;}
    slot->insert( a, key );
    a->second = item;

    //tbb::spin_mutex::scoped_lock lock;
    ////lock.acquire(my_mutex);
    //printf("\nWhiteboard - published product: '%s'\n", key_name );
    //lock.release();

}

// TODO: this operation is *not* thread safe
void Whiteboard::print_slot_content(const unsigned  int slot_number) const {
    printf("++++++++++++++++++++++++\nContent of slot %i:\n", slot_number);
    StringDataMap::const_iterator i;
    StringDataMap* slot = slots_[slot_number];
    for( i=slot->begin(); i!=slot->end(); ++i ){
        printf("\t %s :\t %i\n",i->first.c_str(),i->second);
    }
    printf("++++++++++++++++++++++++\n");
}

bool Whiteboard::read(DataItem& item, const std::string& label, const unsigned int slot_number) const {
  StringDataMap::const_accessor a; 
  StringDataMap* slot = slots_[slot_number]; 
  bool successful = slot->find(a, label);
  if (successful){
    item = a->second; 
    //printf("Whiteboard - reading of %s successful: %i \n", label.c_str(), item);
  } else {
    printf("Whiteboard - reading of %s failed.\n", label.c_str());
  }

  return successful; 
}

bool Whiteboard::get_context(Context*& context){
    // which context is free?
    for (unsigned int i=0, max=contexts_.size(); i<max; ++i) {
        if (contexts_[i].second == available) {
            context = contexts_[i].first;
            contexts_[i].second = in_use;
            return true;
        }
    }
    return false;
}

void Whiteboard::release_context(Context*& context){
    // TODO: for now slot and context number are identical
    // will change in the future
    const unsigned int& i = context->get_slotnumber();
    context->reset();
    slots_[i]->clear();
    contexts_[i].second = available; 

}

unsigned int Whiteboard::register_dependency(const std::string& label){
    std::map<std::string, unsigned int>::iterator i(data_id_map_.find(label));
    if (i != data_id_map_.end()) {
        return i->second;         
    } else {
        unsigned int counter = data_id_counter_++;
        data_id_map_[name_] = data_id_counter_;
        return counter; 
    }
}
