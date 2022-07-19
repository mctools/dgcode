#ifndef BOOST_SERIALIZATION_STATE_SAVER_HPP
#define BOOST_SERIALIZATION_STATE_SAVER_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// state_saver.hpp:

// (C) Copyright 2003-4 Pavel Vozenilek and Robert Ramey - http://www.rrsd.com.
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/serialization for updates, documentation, and revision history.

// Inspired by Daryle Walker's iostate_saver concept.  This saves the original
// value of a variable when a state_saver is constructed and restores
// upon destruction.  Useful for being sure that state is restored to
// variables upon exit from scope.


#include <dgboost/config.hpp>
#ifndef BOOST_NO_EXCEPTIONS
    #include <exception>
#endif

#include <dgboost/call_traits.hpp>
#include <dgboost/noncopyable.hpp>
#include <dgboost/type_traits/has_nothrow_copy.hpp>
#include <dgboost/core/no_exceptions_support.hpp>

#include <dgboost/mpl/eval_if.hpp>
#include <dgboost/mpl/identity.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost {
namespace serialization {

template<class T>
// T requirements:
//  - POD or object semantic (cannot be reference, function, ...)
//  - copy constructor
//  - operator = (no-throw one preferred)
class state_saver : private dgboost::noncopyable
{
private:
    const T previous_value;
    T & previous_ref;

    struct restore {
        static void invoke(T & previous_ref, const T & previous_value){
            previous_ref = previous_value; // won't throw
        }
    };

    struct restore_with_exception {
        static void invoke(T & previous_ref, const T & previous_value){
            BOOST_TRY{
                previous_ref = previous_value;
            } 
            BOOST_CATCH(::std::exception &) { 
                // we must ignore it - we are in destructor
            }
            BOOST_CATCH_END
        }
    };

public:
    state_saver(
        T & object
    ) : 
        previous_value(object),
        previous_ref(object) 
    {}
    
    ~state_saver() {
        #ifndef BOOST_NO_EXCEPTIONS
            typedef typename mpl::eval_if<
                has_nothrow_copy< T >,
                mpl::identity<restore>,
                mpl::identity<restore_with_exception>
            >::type typex;
            typex::invoke(previous_ref, previous_value);
        #else
            previous_ref = previous_value;
        #endif
    }

}; // state_saver<>

} // serialization
} // boost

#endif //BOOST_SERIALIZATION_STATE_SAVER_HPP
