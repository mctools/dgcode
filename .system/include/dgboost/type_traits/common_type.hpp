#ifndef BOOST_TYPE_TRAITS_COMMON_TYPE_HPP_INCLUDED
#define BOOST_TYPE_TRAITS_COMMON_TYPE_HPP_INCLUDED

//
//  Copyright 2015 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//

#include <dgboost/config.hpp>
#include <dgboost/type_traits/decay.hpp>
#include <dgboost/type_traits/declval.hpp>
#include <dgboost/detail/workaround.hpp>
#include <dgboost/type_traits/is_complete.hpp>
#include <dgboost/type_traits/is_void.hpp>
#include <dgboost/type_traits/is_array.hpp>
#include <dgboost/static_assert.hpp>

#if defined(BOOST_NO_CXX11_DECLTYPE)
#include <dgboost/type_traits/detail/common_type_impl.hpp>
#endif

#if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES) && !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)
#include <dgboost/type_traits/detail/mp_defer.hpp>
#endif

namespace dgboost {} namespace boost = dgboost; namespace dgboost
{

// variadic common_type

#if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

template<class... T> struct common_type
{
};

#if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)

template<class... T> using common_type_t = typename common_type<T...>::type;

namespace type_traits_detail
{

template<class T1, class T2, class... T> using common_type_fold = common_type_t<common_type_t<T1, T2>, T...>;

} // namespace type_traits_detail

template<class T1, class T2, class... T>
struct common_type<T1, T2, T...>: type_traits_detail::mp_defer<type_traits_detail::common_type_fold, T1, T2, T...>
{
};

#else

template<class T1, class T2, class... T>
struct common_type<T1, T2, T...>: common_type<typename common_type<T1, T2>::type, T...>
{
};

#endif // !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)

#else

template<
    class T1 = void, class T2 = void, class T3 = void,
    class T4 = void, class T5 = void, class T6 = void,
    class T7 = void, class T8 = void, class T9 = void
>
struct common_type: common_type<typename common_type<T1, T2>::type, T3, T4, T5, T6, T7, T8, T9>
{
};

#endif // !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

// one argument

template<class T> struct common_type<T>: dgboost::decay<T>
{
   BOOST_STATIC_ASSERT_MSG(::dgboost::is_complete<T>::value || ::dgboost::is_void<T>::value || ::dgboost::is_array<T>::value, "Arguments to common_type must both be complete types");
};

// two arguments

namespace type_traits_detail
{

// binary common_type

#if !defined(BOOST_NO_CXX11_DECLTYPE)

#if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES) && !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)
    
#if !defined(BOOST_MSVC) || BOOST_MSVC > 1800

// internal compiler error on msvc-12.0

template<class T1, class T2> using builtin_common_type = typename dgboost::decay<decltype( dgboost::declval<bool>()? dgboost::declval<T1>(): dgboost::declval<T2>() )>::type;

template<class T1, class T2> struct common_type_impl: mp_defer<builtin_common_type, T1, T2>
{
};

#else

template<class T1, class T2> using builtin_common_type = decltype( dgboost::declval<bool>()? dgboost::declval<T1>(): dgboost::declval<T2>() );

template<class T1, class T2> struct common_type_impl_2: mp_defer<builtin_common_type, T1, T2>
{
};

template<class T1, class T2> using decay_common_type = typename dgboost::decay<typename common_type_impl_2<T1, T2>::type>::type;

template<class T1, class T2> struct common_type_impl: mp_defer<decay_common_type, T1, T2>
{
};

#endif // !defined(BOOST_MSVC) || BOOST_MSVC > 1800

#else

template<class T1, class T2> struct common_type_impl: dgboost::decay<decltype( dgboost::declval<bool>()? dgboost::declval<T1>(): dgboost::declval<T2>() )>
{
};

#endif // #if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES) && !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

#endif // #if !defined(BOOST_NO_CXX11_DECLTYPE)

// decay helper

template<class T1, class T2, class T1d = typename dgboost::decay<T1>::type, class T2d = typename dgboost::decay<T2>::type> struct common_type_decay_helper: dgboost::common_type<T1d, T2d>
{
};

template<class T1, class T2> struct common_type_decay_helper<T1, T2, T1, T2>: common_type_impl<T1, T2>
{
};

} // type_traits_detail

template<class T1, class T2> struct common_type<T1, T2>: type_traits_detail::common_type_decay_helper<T1, T2>
{
   BOOST_STATIC_ASSERT_MSG(::dgboost::is_complete<T1>::value || ::dgboost::is_void<T1>::value || ::dgboost::is_array<T1>::value, "Arguments to common_type must both be complete types");
   BOOST_STATIC_ASSERT_MSG(::dgboost::is_complete<T2>::value || ::dgboost::is_void<T2>::value || ::dgboost::is_array<T2>::value, "Arguments to common_type must both be complete types");
};

} // namespace dgboost

#endif // #ifndef BOOST_TYPE_TRAITS_COMMON_TYPE_HPP_INCLUDED
