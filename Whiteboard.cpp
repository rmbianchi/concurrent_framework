/**
 * Whiteboard.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: rbianchi@cern.ch  (Riccardo-Maria BIANCHI)
 */

// include internals
#include "Whiteboard.h"


Whiteboard::Whiteboard(const char* name) : my_name(name), my_notifyEnabled(true)  {}

Whiteboard::~Whiteboard() {
	// TODO Auto-generated destructor stub
}

void Whiteboard::insert_into_table ( string &key, MyClass &m ) {

	// create an accessor that will act as a smart pointer
	// for write access
	tbb::concurrent_hash_map<string, MyClass, my_hash_compare>::accessor a;


	// call insert to create a new element, or return an existing
	// element if one exists.
	// accessor a locks this element for exclusive use by this thread
	string_table.insert( a, key );

	const char* key_name = key.c_str();


	// modify the value held by the pair
	a->second = m;
	// the accessor "a" releases the lock on the element when it is
	// destroyed at the end of the scope

	tbb::spin_mutex::scoped_lock lock;
	lock.acquire(my_mutex);
	printf("\nWhiteboard - published product: '%s'\n", key_name );
	lock.release();

	// notify the subscribers about the publication of the product
	notify(key_name);
}



void Whiteboard::notify(const void* what, Subscriber* s)
{
	printf("Whiteboard::notify()\n");
	if (my_notifyEnabled)
	{
		if (my_subscribers.size() == 0) {
			printf("no subscribers to be notified.\n");
		}
		else {
			std::list<Subscriber*>::iterator p;
			for(p = my_subscribers.begin(); p != my_subscribers.end(); p++)
				if (*p != s) {
					printf("\t- updating AlgoChain: %s\n", (*p)->getName());
					(*p)->update(this, what);
				}
		}
	}

	// we set 'notify' to True by default at the end of each notify operation
	my_notifyEnabled = true;
}

