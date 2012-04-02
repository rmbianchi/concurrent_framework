/**
 * Whiteboard.h
 *
 *  Created on: Mar 26, 2012
 *      Author: rbianchi@cern.ch  (Riccardo-Maria BIANCHI)
 */

#ifndef WHITEBOARD_H_
#define WHITEBOARD_H_


// include TBB
#include "tbb/spin_mutex.h"
#include "tbb/concurrent_hash_map.h"
// include c++
#include <list>


using namespace std;


// forward declarations
class Subscriber;
class MyClass {};// dummy class for data objects



/**
 * compare function for hash_map
 */
struct my_hash_compare {
	static size_t hash( const string& x ) {
		size_t h = 0;
		for( const char* s = x.c_str(); *s; ++s )
			h = (h*17)^*s;
		return h;
	}
	//! True if strings are equal
	static bool equal( const string& x, const string& y ) {
		return x==y;
	};
};



/**
 * the Whiteboard class features a concurrent_hash_map to store published products
 * from modules.
 *
 * It also implements the "Publisher" role of the Observer design pattern
 * (aka Publisher/Observer) to notify the AlgoChain classes (the "Subscribers") when
 * a particular product has been published and therefore it can be used
 *
 */
class Whiteboard {
public:
	// methods
	Whiteboard( const char* name);
	virtual ~Whiteboard();
	void insert_into_table ( string &key, MyClass &m );
	const char* getName() {return my_name;};
	// implement the "Publisher" features
	void subscribe(Subscriber* s) { my_subscribers.push_back(s); }
	void unsubscribe(Subscriber* s) { my_subscribers.remove(s); }
	void notify(const void* what = 0, Subscriber *s = 0);
	void setNotifyEnabled(bool flag) { my_notifyEnabled = flag; }
	bool getNotifyEnabled() const { return my_notifyEnabled; }

private:
	tbb::concurrent_hash_map<string, MyClass, my_hash_compare> string_table;
	std::list<Subscriber*> my_subscribers;
	const char* my_name;
	tbb::spin_mutex my_mutex;
	bool my_notifyEnabled;
};


/**
 * The Subscriber class is an abstract class implementing the "Subscriber" role of the
 * Observer design pattern. The class Algo Chain inherits from it to be notified by
 * the Whiteboard class (the "Publisher") when products have been published in the
 * data store.
 */
class Subscriber {
public:
	virtual void update(Whiteboard* who, const void* what = 0) = 0;
	virtual const char* getName() = 0;
};






#endif /* WHITEBOARD_H_ */
