// Copyright David Abrahams 2002.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef INSTANCE_DWA200295_HPP
# define INSTANCE_DWA200295_HPP

# include <dgboost/python/detail/prefix.hpp>

# include <cstddef>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python
{
  struct instance_holder;
}} // namespace dgboost::python

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python { namespace objects { 

// Each extension instance will be one of these
template <class Data = char>
struct instance
{
    PyObject_VAR_HEAD
    PyObject* dict;
    PyObject* weakrefs; 
    instance_holder* objects;

    typedef typename dgboost::python::detail::type_with_alignment<
        dgboost::python::detail::alignment_of<Data>::value
    >::type align_t;
          
    union
    {
        align_t align;
        char bytes[sizeof(Data)];
    } storage;
};

template <class Data>
struct additional_instance_size
{
    typedef instance<Data> instance_data;
    typedef instance<char> instance_char;
    BOOST_STATIC_CONSTANT(
        std::size_t, value = sizeof(instance_data)
                           - BOOST_PYTHON_OFFSETOF(instance_char,storage));
};

}}} // namespace dgboost::python::object

#endif // INSTANCE_DWA200295_HPP
