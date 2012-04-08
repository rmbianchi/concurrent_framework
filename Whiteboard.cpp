/**
 * Whiteboard.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: rbianchi@cern.ch  (Riccardo-Maria BIANCHI)
 *              benedikt.hegner@cern.ch  (Benedikt HEGNER)
 */

// include internals
#include "Whiteboard.h"


Whiteboard::Whiteboard(const char* name, const int i) : my_name(name), number_of_slots(i) {
  for (int i=0; i<=number_of_slots; ++i){
    m_contexts.push_back(new Context(i,*this));
    m_slots.push_back(new StringDataMap());
  }

}

Whiteboard::~Whiteboard() {
  //TODO: delete the slots
}

void Whiteboard::write ( const DataItem &item, const std::string &key, const int slot_number) {

       // use accessor as a cursor in the concurrent data structure
       // releases lock at destruction of accessor 
       StringDataMap::accessor a;
       StringDataMap* slot = m_slots[slot_number];
       slot->insert( a, key );
       a->second = item;

	//tbb::spin_mutex::scoped_lock lock;
	////lock.acquire(my_mutex);
	//printf("\nWhiteboard - published product: '%s'\n", key_name );
	//lock.release();

}

// TODO: this operation is *not* thread safe
void Whiteboard::print_slot_content(const int slot_number) const {
    printf("++++++++++++++++++++++++\nContent of slot %i:\n", slot_number);
    StringDataMap::const_iterator i;
    StringDataMap* slot = m_slots[slot_number];
    for( i=slot->begin(); i!=slot->end(); ++i ){
        printf("\t %s :\t %i\n",i->first.c_str(),i->second);
    }
    printf("++++++++++++++++++++++++\n");
}

bool Whiteboard::read(DataItem& item, const std::string& label, const int slot_number) const {
  StringDataMap::const_accessor a; 
  StringDataMap* slot = m_slots[slot_number]; 
  bool successful = slot->find(a, label);
  if (successful){
    item =a->second; 
    //printf("Whiteboard - reading of %s successful: %i \n", label.c_str(), item);
  } else {
    printf("Whiteboard - reading of %s failed.\n", label.c_str());
  }

  return successful; 
}

//TODO: safe guard it against exceeded range!
Context* Whiteboard::getContext(const int i){
  return m_contexts[i];
}
