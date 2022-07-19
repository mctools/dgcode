//  (C) Copyright 2009-2011 Frederic Bron.
//
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef BOOST_TT_HAS_PLUS_ASSIGN_HPP_INCLUDED
#define BOOST_TT_HAS_PLUS_ASSIGN_HPP_INCLUDED

#include <dgboost/config.hpp>
#include <dgboost/type_traits/detail/config.hpp>

// cannot include this header without getting warnings of the kind:
// gcc:
//    warning: value computed is not used
//    warning: comparison between signed and unsigned integer expressions
// msvc:
//    warning C4018: '<' : signed/unsigned mismatch
//    warning C4244: '+=' : conversion from 'double' to 'char', possible loss of data
//    warning C4547: '*' : operator before comma has no effect; expected operator with side-effect
//    warning C4800: 'int' : forcing value to bool 'true' or 'false' (performance warning)
//    warning C4804: '<' : unsafe use of type 'bool' in operation
//    warning C4805: '==' : unsafe mix of type 'bool' and type 'char' in operation
// cannot find another implementation -> declared as system header to suppress these warnings.
#if defined(__GNUC__)
#   pragma GCC system_header
#elif defined(BOOST_MSVC)
#   pragma warning ( push )
#   pragma warning ( disable : 4018 4244 4547 4800 4804 4805 4913 4133)
#   if BOOST_WORKAROUND(BOOST_MSVC_FULL_VER, >= 140050000)
#       pragma warning ( disable : 6334)
#   endif
#endif

#if defined(BOOST_TT_HAS_ACCURATE_BINARY_OPERATOR_DETECTION)

#include <dgboost/type_traits/integral_constant.hpp>
#include <dgboost/type_traits/make_void.hpp>
#include <dgboost/type_traits/is_convertible.hpp>
#include <dgboost/type_traits/is_void.hpp>
#include <dgboost/type_traits/is_same.hpp>
#include <dgboost/type_traits/is_pointer.hpp>
#include <dgboost/type_traits/is_arithmetic.hpp>
#include <dgboost/type_traits/add_reference.hpp>
#include <dgboost/type_traits/remove_pointer.hpp>
#include <dgboost/type_traits/remove_reference.hpp>
#include <dgboost/type_traits/remove_cv.hpp>
#include <utility>

namespace dgboost {} namespace boost = dgboost; namespace dgboost
{

   namespace binary_op_detail {

      struct dont_care;

      template <class T, class U, class Ret, class = void>
      struct has_plus_assign_ret_imp : public dgboost::false_type {};

      template <class T, class U, class Ret>
      struct has_plus_assign_ret_imp<T, U, Ret, typename dgboost::make_void<decltype(std::declval<typename add_reference<T>::type>() += std::declval<typename add_reference<U>::type>())>::type>
         : public dgboost::integral_constant<bool, ::dgboost::is_convertible<decltype(std::declval<typename add_reference<T>::type>() += std::declval<typename add_reference<U>::type>()), Ret>::value> {};

      template <class T, class U, class = void >
      struct has_plus_assign_void_imp : public dgboost::false_type {};

      template <class T, class U>
      struct has_plus_assign_void_imp<T, U, typename dgboost::make_void<decltype(std::declval<typename add_reference<T>::type>() += std::declval<typename add_reference<U>::type>())>::type>
         : public dgboost::integral_constant<bool, ::dgboost::is_void<decltype(std::declval<typename add_reference<T>::type>() += std::declval<typename add_reference<U>::type>())>::value> {};

      template <class T, class U, class = void>
      struct has_plus_assign_dc_imp : public dgboost::false_type {};

      template <class T, class U>
      struct has_plus_assign_dc_imp<T, U, typename dgboost::make_void<decltype(std::declval<typename add_reference<T>::type>() += std::declval<typename add_reference<U>::type>())>::type>
         : public dgboost::true_type {};

      template <class T, class U, class Ret>
      struct has_plus_assign_filter_ret : public dgboost::binary_op_detail:: has_plus_assign_ret_imp <T, U, Ret> {};
      template <class T, class U>
      struct has_plus_assign_filter_ret<T, U, void> : public dgboost::binary_op_detail:: has_plus_assign_void_imp <T, U> {};
      template <class T, class U>
      struct has_plus_assign_filter_ret<T, U, dgboost::binary_op_detail::dont_care> : public dgboost::binary_op_detail:: has_plus_assign_dc_imp <T, U> {};

      template <class T, class U, class Ret, bool f>
      struct has_plus_assign_filter_impossible : public dgboost::binary_op_detail:: has_plus_assign_filter_ret <T, U, Ret> {};
      template <class T, class U, class Ret>
      struct has_plus_assign_filter_impossible<T, U, Ret, true> : public dgboost::false_type {};

   }

   template <class T, class U = T, class Ret = dgboost::binary_op_detail::dont_care>
   struct has_plus_assign : public dgboost::binary_op_detail:: has_plus_assign_filter_impossible <T, U, Ret, dgboost::is_arithmetic<typename dgboost::remove_reference<T>::type>::value && dgboost::is_pointer<typename remove_reference<U>::type>::value && !dgboost::is_same<bool, typename dgboost::remove_cv<typename remove_reference<T>::type>::type>::value> {};

}

#else

#define BOOST_TT_TRAIT_NAME has_plus_assign
#define BOOST_TT_TRAIT_OP +=
#define BOOST_TT_FORBIDDEN_IF\
   (\
      /* Lhs==pointer and Rhs==pointer */\
      (\
         ::dgboost::is_pointer< Lhs_noref >::value && \
         ::dgboost::is_pointer< Rhs_noref >::value\
      ) || \
      /* Lhs==void* and Rhs==fundamental */\
      (\
         ::dgboost::is_pointer< Lhs_noref >::value && \
         ::dgboost::is_void< Lhs_noptr >::value && \
         ::dgboost::is_fundamental< Rhs_nocv >::value\
      ) || \
      /* Rhs==void* and Lhs==fundamental */\
      (\
         ::dgboost::is_pointer< Rhs_noref >::value && \
         ::dgboost::is_void< Rhs_noptr >::value && \
         ::dgboost::is_fundamental< Lhs_nocv >::value\
      ) || \
      /* Lhs==pointer and Rhs==fundamental and Rhs!=integral */\
      (\
         ::dgboost::is_pointer< Lhs_noref >::value && \
         ::dgboost::is_fundamental< Rhs_nocv >::value && \
         (!  ::dgboost::is_integral< Rhs_noref >::value )\
      ) || \
      /* Rhs==pointer and Lhs==fundamental and Lhs!=bool */\
      (\
         ::dgboost::is_pointer< Rhs_noref >::value && \
         ::dgboost::is_fundamental< Lhs_nocv >::value && \
         (!  ::dgboost::is_same< Lhs_nocv, bool >::value )\
      ) || \
      /* (Lhs==fundamental or Lhs==pointer) and (Rhs==fundamental or Rhs==pointer) and (Lhs==const) */\
      (\
         (\
            ::dgboost::is_fundamental< Lhs_nocv >::value || \
            ::dgboost::is_pointer< Lhs_noref >::value\
          ) && \
         ( \
            ::dgboost::is_fundamental< Rhs_nocv >::value || \
            ::dgboost::is_pointer< Rhs_noref >::value\
          ) && \
         ::dgboost::is_const< Lhs_noref >::value\
      )\
      )


#include <dgboost/type_traits/detail/has_binary_operator.hpp>

#undef BOOST_TT_TRAIT_NAME
#undef BOOST_TT_TRAIT_OP
#undef BOOST_TT_FORBIDDEN_IF

#endif

#if defined(BOOST_MSVC)
#   pragma warning (pop)
#endif

#endif
