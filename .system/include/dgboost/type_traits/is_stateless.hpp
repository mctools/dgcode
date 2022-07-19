
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef BOOST_TT_IS_STATELESS_HPP_INCLUDED
#define BOOST_TT_IS_STATELESS_HPP_INCLUDED

#include <dgboost/type_traits/has_trivial_constructor.hpp>
#include <dgboost/type_traits/has_trivial_copy.hpp>
#include <dgboost/type_traits/has_trivial_destructor.hpp>
#include <dgboost/type_traits/is_class.hpp>
#include <dgboost/type_traits/is_empty.hpp>
#include <dgboost/config.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost {

template <typename T>
struct is_stateless
 : public integral_constant<bool,  
      (::dgboost::has_trivial_constructor<T>::value
      && ::dgboost::has_trivial_copy<T>::value
      && ::dgboost::has_trivial_destructor<T>::value
      && ::dgboost::is_class<T>::value
      && ::dgboost::is_empty<T>::value)>
{};

} // namespace dgboost

#endif // BOOST_TT_IS_STATELESS_HPP_INCLUDED
