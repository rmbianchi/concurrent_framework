/**
 * Context.cpp
 *
 *  Created on: Apr 4, 2012
 *      Author: benedikt.hegner@cern.ch  (Benedikt HEGNER)
 */

// FWK includes 
#include "Context.h"
#include "Whiteboard.h"

Context::Context(const int i, Whiteboard& wb) : slotnumber_(i),  finished_(false), wb_(wb) {}
Context::~Context() {}

void Context::write ( const DataItem& item, const std::string &algo_name, const std::string &label ) {
    wb_.write(item,label, slotnumber_);
    //wb->write(item,algo_name+":"+label,_number);
}

void Context::print_content() const {
    wb_.print_slot_content(slotnumber_);
}

bool Context::read(DataItem& item, const std::string& label) const {
  return wb_.read(item, label, slotnumber_);
}

void Context::reset(){
    finished_ = false;
}
