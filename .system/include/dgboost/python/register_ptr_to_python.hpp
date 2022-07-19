// Copyright David Abrahams 2002.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef REGISTER_PTR_TO_PYTHON_HPP
#define REGISTER_PTR_TO_PYTHON_HPP

#include <dgboost/python/pointee.hpp>
#include <dgboost/python/object.hpp>
#include <dgboost/python/object/class_wrapper.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python {
    
template <class P>
void register_ptr_to_python()
{
    typedef typename dgboost::python::pointee<P>::type X;
    objects::class_value_wrapper<
        P
      , objects::make_ptr_instance<
            X
          , objects::pointer_holder<P,X>
        >
    >();
}           

}} // namespace dgboost::python

#endif // REGISTER_PTR_TO_PYTHON_HPP


