To Compile:
===========

```bash
cmake -G Xcode -DTBB_INSTALL_DIR=/opt/intel/tbb/tbb40_297oss
```
... and then fire up XCode

OR

```bash
cmake -DTBB_INSTALL_DIR=/opt/intel/tbb/tbb40_297oss
make
```

About:
======

This project provides a concurrent whiteboard and a few implementations of parallel algorithm/module scheduling for HEP on top of Intel's TBB:

 1) ```dependency_graph.cpp``` uses ```tbb::flow::graph```. A few additions to TBB were necessary
 2) ```taskgraph_example.cpp``` uses ```tbb:task``` directly and uses N copies of the algorithm dependency DAG to handle N events in parallel
 3) ```taskbits_example.cpp``` uses ```tbb:task``` directly and uses each one bit per algorithm to descripe the state of the event. 
 
The implementations 2) and 3) provide a feature to deal with limited resources/ non-reentrant algorithms. 


PLEASE NOTICE: TBB bugfix!!! 
============================

(Still valid with TBB 4.0 Update 4)

Including `#include "tbb/flow_graph.h"` in more than one file in the same project will lead to Linker error about "duplicate symbol".

To fix it, you have to edit the ```flow_graph.h``` header file in the TBB distribution you installed on your machine.
Please follow instructions from the official TBB Forum at:
http://software.intel.com/en-us/forums/showthread.php?t=103695

In two words: you have to edit the header file to add an "inline" declaration at 2 methods, i.e. changing these two lines:
```cpp
void register_node(graph_node *n); 
void remove_node(graph_node *n);
```

into these:
```cpp
inline void register_node(graph_node *n); 
inline void remove_node(graph_node *n);
```

The bugfix will be included in one of the next TBB releases.





