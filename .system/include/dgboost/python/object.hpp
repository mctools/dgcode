// Copyright David Abrahams 2002.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef OBJECT_DWA2002612_HPP
# define OBJECT_DWA2002612_HPP

# include <dgboost/python/ssize_t.hpp>
# include <dgboost/python/object_core.hpp>
# include <dgboost/python/object_attributes.hpp>
# include <dgboost/python/object_items.hpp>
# include <dgboost/python/object_slices.hpp>
# include <dgboost/python/object_operators.hpp>
# include <dgboost/python/converter/arg_to_python.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python {

    inline ssize_t len(object const& obj)
    {
        ssize_t result = PyObject_Length(obj.ptr());
        if (PyErr_Occurred()) throw_error_already_set();
        return result;
    }

}} // namespace dgboost::python

#endif // OBJECT_DWA2002612_HPP
