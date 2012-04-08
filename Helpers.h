//
//  Helpers.h
//  
//
//  Created by Benedikt Hegner on 4/8/12.
//  Copyright (c) 2012 __CERN__. All rights reserved.
//

#ifndef _Helpers_h
#define _Helpers_h

//===========================
//		timing tools
//===========================
typedef unsigned long long timestamp_t;
static timestamp_t get_timestamp () {
    struct timeval now;
    gettimeofday (&now, NULL);
    return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}

#endif
