// Copyright David Abrahams 2003.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef COPY_CTOR_MUTATES_RHS_DWA2003219_HPP
# define COPY_CTOR_MUTATES_RHS_DWA2003219_HPP

#include <dgboost/python/detail/is_auto_ptr.hpp>
#include <dgboost/mpl/bool.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python { namespace detail { 

template <class T>
struct copy_ctor_mutates_rhs
    : is_auto_ptr<T>
{
};

}}} // namespace dgboost::python::detail

#endif // COPY_CTOR_MUTATES_RHS_DWA2003219_HPP
