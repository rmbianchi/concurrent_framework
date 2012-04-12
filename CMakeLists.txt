cmake_minimum_required(VERSION 2.8.3 FATAL_ERROR)

project(CF4Hep)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake) 

#---Options-------------------------------------------------------------------------
find_package(TBB REQUIRED)

#---Packages------------------------------------------------------------------------

#--Examples-------------------------------------------------------------------------
include_directories(${TBB_INCLUDE_DIR})

add_executable(CF4HepTaskBitsDemonstrator taskbits_example.cpp Whiteboard.cpp Context.cpp Scheduler.cpp AlgoPool.cpp EventLoopManager.cpp)
target_link_libraries(CF4HepTaskBitsDemonstrator ${TBB_LIBRARIES})

#---Configuration-------------------------------------------------------------------
configure_file(cmake/thisCF4Hep.csh thisCF4Hep.csh @ONLY)
#configure_file(cmake/thisdd4hep.sh thisdd4hep.sh @ONLY)

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${TBB_LIBRARY_DIR})
