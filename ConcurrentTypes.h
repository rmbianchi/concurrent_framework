/**
 * ConcurrentTypes.h
 *
 *  Created on: Feb 12, 2012
 *      Author: benedikt.hegner@cern.ch  (Benedikt HEGNER)
 */

#ifndef CONCURRENTTYPES_H_
#define CONCURRENTTYPES_H_

// include c++
#include <string>
// include tbb
#include "tbb/concurrent_hash_map.h"

// use unsigned int as DataItem
typedef unsigned int DataItem;

// Hash&Compare functions for various types used
// TODO: replace the string hash by a better implementation, murmur etc.
struct StringHashCompare {
  static size_t hash( const std::string& x ) {
    size_t h = 0;
    for( const char* s = x.c_str(); *s; ++s )
      h = (h*17)^*s;
    return h;
  }
  static bool equal( const std::string& x, const std::string& y ) {
    return x==y;
  }
};

typedef tbb::concurrent_hash_map<std::string, unsigned int, StringHashCompare> StringUIntMap;
typedef tbb::concurrent_hash_map<std::string, int, StringHashCompare> StringIntMap;
typedef tbb::concurrent_hash_map<std::string, DataItem, StringHashCompare> StringDataMap;

#endif /* CONCURRENTTYPES_H_ */
