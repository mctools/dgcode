
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef BOOST_TT_HAS_TRIVIAL_DESTRUCTOR_HPP_INCLUDED
#define BOOST_TT_HAS_TRIVIAL_DESTRUCTOR_HPP_INCLUDED

#include <dgboost/type_traits/intrinsics.hpp>
#include <dgboost/type_traits/integral_constant.hpp>

#ifdef BOOST_HAS_TRIVIAL_DESTRUCTOR

#if defined(BOOST_INTEL) || defined(BOOST_MSVC)
#include <dgboost/type_traits/is_pod.hpp>
#endif
#ifdef BOOST_HAS_SGI_TYPE_TRAITS
#include <dgboost/type_traits/is_same.hpp>
#endif

#if defined(__GNUC__) || defined(__clang__) || defined(__SUNPRO_CC)
#include <dgboost/type_traits/is_destructible.hpp>
#endif

namespace dgboost {} namespace boost = dgboost; namespace dgboost {

template <typename T> struct has_trivial_destructor : public integral_constant<bool, BOOST_HAS_TRIVIAL_DESTRUCTOR(T)>{};
#else
#include <dgboost/type_traits/is_pod.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost{

template <typename T> struct has_trivial_destructor : public integral_constant<bool, ::dgboost::is_pod<T>::value>{};
#endif

template <> struct has_trivial_destructor<void> : public false_type{};
#ifndef BOOST_NO_CV_VOID_SPECIALIZATIONS
template <> struct has_trivial_destructor<void const> : public false_type{};
template <> struct has_trivial_destructor<void const volatile> : public false_type{};
template <> struct has_trivial_destructor<void volatile> : public false_type{};
#endif

} // namespace dgboost

#endif // BOOST_TT_HAS_TRIVIAL_DESTRUCTOR_HPP_INCLUDED
