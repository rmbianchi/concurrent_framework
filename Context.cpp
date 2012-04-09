/**
 * Context.cpp
 *
 *  Created on: Apr 4, 2012
 *      Author: benedikt.hegner@cern.ch  (Benedikt HEGNER)
 */

// FWK includes 
#include "Context.h"
#include "Whiteboard.h"

Context::Context(const int i, Whiteboard& wb) : _number(i),  m_finished(false), wb(wb) {}
Context::~Context() {}

void Context::write ( const DataItem& item, const std::string &algo_name, const std::string &label ) {
    wb.write(item,label,_number);
    //wb->write(item,algo_name+":"+label,_number);
}

void Context::print_content() const {
    wb.print_slot_content(_number);
}

bool Context::read(DataItem& item, const std::string& label) const {
  return wb.read(item, label, _number);
}
