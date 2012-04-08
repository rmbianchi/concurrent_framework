//
//  TaskSchedule.h
//  
//
//  Created by Benedikt Hegner on 4/6/12.
//  Copyright (c) 2012 __CERN__. All rights reserved.
//

#ifndef _TaskSchedule_h
#define _TaskSchedule_h

// include tbb
#include <task.h>


/**
 * Task class wrapping around AlgoBase.h
 */
class AlgoTask :: public tbb:task{}

/**
 * Scheduling class based on bare tbb::task
 */
class TaskSchedule {};


#endif
