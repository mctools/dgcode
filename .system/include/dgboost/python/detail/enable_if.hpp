// Copyright David Abrahams 2004. Distributed under the Boost
// Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef ENABLE_IF_DWA2004722_HPP
# define ENABLE_IF_DWA2004722_HPP

# include <dgboost/python/detail/sfinae.hpp>
# include <dgboost/detail/workaround.hpp>

#if !defined(BOOST_NO_SFINAE)
#  include <dgboost/utility/enable_if.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python { namespace detail { 

template <class C, class T = int>
struct enable_if_arg
  : enable_if<C,T>
{};
             
template <class C, class T = int>
struct disable_if_arg
  : disable_if<C,T>
{};
             
template <class C, class T = void>
struct enable_if_ret
  : enable_if<C,T>
{};
             
template <class C, class T = void>
struct disable_if_ret
  : disable_if<C,T>
{};
             
}}} // namespace dgboost::python::detail

# endif

#endif // ENABLE_IF_DWA2004722_HPP
