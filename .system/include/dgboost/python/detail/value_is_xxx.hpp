// Copyright David Abrahams 2003.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef VALUE_IS_XXX_DWA2003224_HPP
# define VALUE_IS_XXX_DWA2003224_HPP

# include <dgboost/config.hpp>
# include <dgboost/mpl/bool.hpp>
# include <dgboost/preprocessor/enum_params.hpp>

# include <dgboost/python/detail/type_traits.hpp>
#  include <dgboost/python/detail/is_xxx.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python { namespace detail {

#  define BOOST_PYTHON_VALUE_IS_XXX_DEF(name, qualified_name, nargs)    \
template <class X_>                                                     \
struct value_is_##name                                                  \
{                                                                       \
    BOOST_PYTHON_IS_XXX_DEF(name,qualified_name,nargs)                  \
    BOOST_STATIC_CONSTANT(bool, value = is_##name<                      \
                               typename remove_cv<                      \
                                  typename remove_reference<X_>::type   \
                               >::type                                  \
                           >::value);                                   \
    typedef mpl::bool_<value> type;                                     \
                                                                        \
};                                                              

}}} // namespace dgboost::python::detail

#endif // VALUE_IS_XXX_DWA2003224_HPP
