// Copyright David Abrahams 2004. Distributed under the Boost
// Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef UNWRAP_WRAPPER_DWA2004723_HPP
# define UNWRAP_WRAPPER_DWA2004723_HPP

# include <dgboost/python/detail/prefix.hpp>
# include <dgboost/python/detail/is_wrapper.hpp>
# include <dgboost/mpl/eval_if.hpp>
# include <dgboost/mpl/identity.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python { namespace detail { 

template <class T>
struct unwrap_wrapper_helper
{
    typedef typename T::_wrapper_wrapped_type_ type;
};

template <class T>
struct unwrap_wrapper_
  : mpl::eval_if<is_wrapper<T>,unwrap_wrapper_helper<T>,mpl::identity<T> >
{};

template <class T>
typename unwrap_wrapper_<T>::type*
unwrap_wrapper(T*)
{
    return 0;
}

}}} // namespace dgboost::python::detail

#endif // UNWRAP_WRAPPER_DWA2004723_HPP
