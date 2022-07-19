// Copyright David Abrahams 2002.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef REGISTERED_POINTEE_DWA2002710_HPP
# define REGISTERED_POINTEE_DWA2002710_HPP
# include <dgboost/python/converter/registered.hpp>
# include <dgboost/python/converter/pointer_type_id.hpp>
# include <dgboost/python/converter/registry.hpp>
# include <dgboost/python/detail/type_traits.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python { namespace converter { 

struct registration;

template <class T>
struct registered_pointee
    : registered<
        typename dgboost::python::detail::remove_pointer<
           typename dgboost::python::detail::remove_cv<
              typename dgboost::python::detail::remove_reference<T>::type
           >::type
        >::type
    >
{
};
}}} // namespace dgboost::python::converter

#endif // REGISTERED_POINTEE_DWA2002710_HPP
