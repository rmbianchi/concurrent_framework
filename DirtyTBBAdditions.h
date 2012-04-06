/**
 * DirtyTBBAdditions.h
 *
 *  Created on: Apr 5, 2012
 *      Author: benedikt.hegner@cern.ch  (Benedikt HEGNER)
 */

#ifndef DIRTYTBBADDITIONS_H_
#define DIRTYTBBADDITIONS_H_

#include "tbb/flow_graph.h"

// forward declaration
class Context;

namespace tbb{
    namespace flow{
        namespace interface6{
            namespace internal{

                //! A special template instantiation for the cache of successors
                template<>
                class successor_cache<Context*, spin_rw_mutex> : tbb::internal::no_copy {
                protected:
                    typedef Context* T;
                    typedef spin_rw_mutex my_mutex_type;
                    my_mutex_type my_mutex;
                    
                    typedef std::list< receiver<T> * > my_successors_type;
                    my_successors_type my_successors;
                    
                    sender<T> *my_owner;
                    
                public:
                    
                    successor_cache( ) : my_owner(NULL) {}
                    
                    void set_owner( sender<T> *owner ) { my_owner = owner; }
                    
                    virtual ~successor_cache() {}
                    
                    void register_successor( receiver<T> &r ) {
                        typename my_mutex_type::scoped_lock l(my_mutex, true);
                        my_successors.push_back( &r ); 
                        if ( my_owner ) {
                            receiver<T> *cr = dynamic_cast< receiver<T> * >(&r);
                            if ( cr )
                                cr->register_predecessor( *my_owner );
                        }
                        
                    }
                    
                    void remove_successor( receiver<T> &r ) {
                        typename my_mutex_type::scoped_lock l(my_mutex, true);
                        for ( typename my_successors_type::iterator i = my_successors.begin();
                             i != my_successors.end(); ++i ) { 
                            if ( *i == & r ) {
                                if ( my_owner )
                                    r.remove_predecessor( *my_owner );
                                my_successors.erase(i);
                                break;
                            }
                        }
                    }
                    
                    bool empty() { 
                        typename my_mutex_type::scoped_lock l(my_mutex, false);
                        return my_successors.empty(); 
                    }
                    
                    virtual bool try_put( const T &t ) = 0; 
                };
            
            } } } }   // closing all the name spaces 

//! new class that implements a Context node which allows multiple inputs
// inspired by continue_receiver
class dirty_new_node : public tbb::flow::function_node<Context*,Context*,tbb::flow::queueing,tbb::cache_aligned_allocator<Context*> >{
    //using graph_node::my_graph;
public:
    typedef tbb::flow::function_node<Context*,Context*,tbb::flow::queueing,tbb::cache_aligned_allocator<Context*> > base_type;
    typedef Context* output_type;
    typedef Context* input_type;
    typedef tbb::flow::sender< input_type > predecessor_type;
    typedef tbb::flow::receiver< output_type > successor_type;
    typedef base_type::fInput_type fInput_type;
    typedef base_type::base_type fInput_base;
    
    //! Constructor
    template< typename Body >
    dirty_new_node( tbb::flow::graph &g, size_t concurrency, Body body ) :
    base_type( g, concurrency, body ),
     my_predecessor_count(0), my_current_count(0)
    {}
    
    //! Copy constructor
    dirty_new_node( const dirty_new_node& src ) :
    base_type(src), my_predecessor_count(src.my_predecessor_count), 
    my_current_count(src.my_current_count)
    {}

    //! Executes the actual code block
    /** If the message causes the message count to reach the predecessor count, the relevant try_put is called and
     the message count is reset to 0.  Otherwise the message count is incremented. */
    virtual bool try_put(const input_type &i) { 
        { 
            tbb::spin_mutex::scoped_lock l(my_mutex);
            if ( ++my_current_count < my_predecessor_count )
                return true; 
            else
                my_current_count = 0;
        }
        return fInput_type::try_put(i);
    }
 
    //! Increments the trigger threshold
    virtual bool register_predecessor( predecessor_type &src ) {
        tbb::spin_mutex::scoped_lock l(my_mutex);
        ++my_predecessor_count;
        return fInput_base::register_predecessor(src);
    }

    //! Decrements the trigger threshold
    /** Does not check to see if the removal of the predecessor now makes the current count
     exceed the new threshold.  So removing a predecessor while the graph is active can cause
     unexpected results. */
    virtual bool remove_predecessor( predecessor_type &src ) {
        tbb::spin_mutex::scoped_lock l(my_mutex);
        --my_predecessor_count;
        return fInput_base::remove_predecessor(src);
    }

protected:
    tbb::spin_mutex my_mutex;
    int my_predecessor_count;
    int my_current_count;
    int my_initial_predecessor_count;
};

#endif /* DIRTYTBBADDITIONS_H_ */
