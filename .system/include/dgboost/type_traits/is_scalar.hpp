
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef BOOST_TT_IS_SCALAR_HPP_INCLUDED
#define BOOST_TT_IS_SCALAR_HPP_INCLUDED

#include <dgboost/type_traits/is_arithmetic.hpp>
#include <dgboost/type_traits/is_enum.hpp>
#include <dgboost/type_traits/is_pointer.hpp>
#include <dgboost/type_traits/is_member_pointer.hpp>
#include <dgboost/config.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost {

template <typename T>
struct is_scalar
   : public integral_constant<bool, ::dgboost::is_arithmetic<T>::value || ::dgboost::is_enum<T>::value || ::dgboost::is_pointer<T>::value || ::dgboost::is_member_pointer<T>::value>
{};

} // namespace dgboost

#endif // BOOST_TT_IS_SCALAR_HPP_INCLUDED
