// Copyright David Abrahams 2003.
// Copyright Stefan Seefeld 2016.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef boost_python_detail_value_is_shared_ptr_hpp_
#define boost_python_detail_value_is_shared_ptr_hpp_

#include <dgboost/python/detail/value_is_xxx.hpp>
#include <dgboost/python/detail/is_shared_ptr.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python { namespace detail { 

template <class X_>
struct value_is_shared_ptr
{
  static bool const value = is_shared_ptr<typename remove_cv<
					    typename remove_reference<X_>
					      ::type>
					    ::type>
    ::value;
  typedef mpl::bool_<value> type;
};

}}} // namespace dgboost::python::detail

#endif // VALUE_IS_SHARED_PTR_DWA2003224_HPP
