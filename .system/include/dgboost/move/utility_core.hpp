//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2012-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/move for documentation.
//
//////////////////////////////////////////////////////////////////////////////

//! \file
//! This header defines core utilities to ease the development
//! of move-aware functions. This header minimizes dependencies
//! from other libraries.

#ifndef BOOST_MOVE_MOVE_UTILITY_CORE_HPP
#define BOOST_MOVE_MOVE_UTILITY_CORE_HPP

#ifndef BOOST_CONFIG_HPP
#  include <dgboost/config.hpp>
#endif
#
#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <dgboost/move/detail/config_begin.hpp>
#include <dgboost/move/detail/workaround.hpp>  //forceinline
#include <dgboost/move/core.hpp>
#include <dgboost/move/detail/meta_utils.hpp>
#include <dgboost/static_assert.hpp>

#if defined(BOOST_NO_CXX11_RVALUE_REFERENCES) && !defined(BOOST_MOVE_DOXYGEN_INVOKED)

   namespace dgboost {} namespace boost = dgboost; namespace dgboost {

   template<class T>
   struct enable_move_utility_emulation
   {
      static const bool value = true;
   };
    
   //////////////////////////////////////////////////////////////////////////////
   //
   //                            move()
   //
   //////////////////////////////////////////////////////////////////////////////

   template <class T>
   BOOST_MOVE_FORCEINLINE typename ::dgboost::move_detail::enable_if_and
      < T &
      , enable_move_utility_emulation<T>
      , has_move_emulation_disabled<T>
      >::type
         move(T& x) BOOST_NOEXCEPT
   {
      return x;
   }

   template <class T>
   BOOST_MOVE_FORCEINLINE typename ::dgboost::move_detail::enable_if_and
      < rv<T>&
      , enable_move_utility_emulation<T>
      , has_move_emulation_enabled<T>
      >::type
         move(T& x) BOOST_NOEXCEPT
   {
      return *BOOST_MOVE_TO_RV_CAST(::dgboost::rv<T>*, ::dgboost::move_detail::addressof(x) );
   }

   template <class T>
   BOOST_MOVE_FORCEINLINE typename ::dgboost::move_detail::enable_if_and
      < rv<T>&
      , enable_move_utility_emulation<T>
      , has_move_emulation_enabled<T>
      >::type
         move(rv<T>& x) BOOST_NOEXCEPT
   {
      return x;
   }

   //////////////////////////////////////////////////////////////////////////////
   //
   //                            forward()
   //
   //////////////////////////////////////////////////////////////////////////////

   template <class T>
   BOOST_MOVE_FORCEINLINE typename ::dgboost::move_detail::enable_if_and
      < T &
      , enable_move_utility_emulation<T>
      , ::dgboost::move_detail::is_rv<T>
      >::type
         forward(const typename ::dgboost::move_detail::identity<T>::type &x) BOOST_NOEXCEPT
   {
      return const_cast<T&>(x);
   }

   template <class T>
   BOOST_MOVE_FORCEINLINE typename ::dgboost::move_detail::enable_if_and
      < const T &
      , enable_move_utility_emulation<T>
      , ::dgboost::move_detail::is_not_rv<T>
      >::type
         forward(const typename ::dgboost::move_detail::identity<T>::type &x) BOOST_NOEXCEPT
   {
      return x;
   }

   //////////////////////////////////////////////////////////////////////////////
   //
   //                        move_if_not_lvalue_reference()
   //
   //////////////////////////////////////////////////////////////////////////////

   template <class T>
   BOOST_MOVE_FORCEINLINE typename ::dgboost::move_detail::enable_if_and
      < T &
      , enable_move_utility_emulation<T>
      , ::dgboost::move_detail::is_rv<T>
      >::type
         move_if_not_lvalue_reference(const typename ::dgboost::move_detail::identity<T>::type &x) BOOST_NOEXCEPT
   {
      return const_cast<T&>(x);
   }

   template <class T>
   BOOST_MOVE_FORCEINLINE typename ::dgboost::move_detail::enable_if_and
      < typename ::dgboost::move_detail::add_lvalue_reference<T>::type
      , enable_move_utility_emulation<T>
      , ::dgboost::move_detail::is_not_rv<T>
      , ::dgboost::move_detail::or_
         < ::dgboost::move_detail::is_lvalue_reference<T>
         , has_move_emulation_disabled<T>
         >
      >::type
         move_if_not_lvalue_reference(typename ::dgboost::move_detail::remove_reference<T>::type &x) BOOST_NOEXCEPT
   {
      return x;
   }

   template <class T>
   BOOST_MOVE_FORCEINLINE typename ::dgboost::move_detail::enable_if_and
      < rv<T>&
      , enable_move_utility_emulation<T>
      , ::dgboost::move_detail::is_not_rv<T>
      , ::dgboost::move_detail::and_
         < ::dgboost::move_detail::not_< ::dgboost::move_detail::is_lvalue_reference<T> >
         , has_move_emulation_enabled<T>
         >
      >::type
         move_if_not_lvalue_reference(typename ::dgboost::move_detail::remove_reference<T>::type &x) BOOST_NOEXCEPT
   {
      return move(x);
   }

   }  //namespace dgboost

#else    //#if defined(BOOST_NO_CXX11_RVALUE_REFERENCES) && !defined(BOOST_MOVE_DOXYGEN_INVOKED)

   #if defined(BOOST_MOVE_USE_STANDARD_LIBRARY_MOVE)
      #include <utility>

      namespace dgboost {} namespace boost = dgboost; namespace dgboost{

      using ::std::move;
      using ::std::forward;

      }  //namespace dgboost

   #else //!BOOST_MOVE_USE_STANDARD_LIBRARY_MOVE

      namespace dgboost {} namespace boost = dgboost; namespace dgboost {

      //! This trait's internal boolean `value` is false in compilers with rvalue references
      //! and true in compilers without rvalue references.
      //!
      //! A user can specialize this trait for a type T to false to SFINAE out `move` and `forward`
      //! so that the user can define a different move emulation for that type in namespace dgboost
      //! (e.g. another Boost library for its types) and avoid any overload ambiguity.
      template<class T>
      struct enable_move_utility_emulation
      {
         static const bool value = false;
      };

      //////////////////////////////////////////////////////////////////////////////
      //
      //                                  move
      //
      //////////////////////////////////////////////////////////////////////////////

      #if defined(BOOST_MOVE_DOXYGEN_INVOKED)
         //! This function provides a way to convert a reference into a rvalue reference
         //! in compilers with rvalue references. For other compilers if `T` is Boost.Move
         //! enabled type then it converts `T&` into <tt>::dgboost::rv<T> &</tt> so that
         //! move emulation is activated, else it returns `T &`.
         template <class T>
         rvalue_reference move(input_reference) noexcept;

      #elif defined(BOOST_MOVE_OLD_RVALUE_REF_BINDING_RULES)

         //Old move approach, lvalues could bind to rvalue references
         template <class T>
         BOOST_MOVE_FORCEINLINE typename ::dgboost::move_detail::remove_reference<T>::type && move(T&& t) BOOST_NOEXCEPT
         {  return t;   }

      #else //BOOST_MOVE_OLD_RVALUE_REF_BINDING_RULES

         template <class T>
         BOOST_MOVE_FORCEINLINE typename ::dgboost::move_detail::remove_reference<T>::type && move(T&& t) BOOST_NOEXCEPT
         { return static_cast<typename ::dgboost::move_detail::remove_reference<T>::type &&>(t); }

      #endif   //BOOST_MOVE_OLD_RVALUE_REF_BINDING_RULES

      //////////////////////////////////////////////////////////////////////////////
      //
      //                                  forward
      //
      //////////////////////////////////////////////////////////////////////////////


      #if defined(BOOST_MOVE_DOXYGEN_INVOKED)
         //! This function provides limited form of forwarding that is usually enough for
         //! in-place construction and avoids the exponential overloading for
         //! achieve the limited forwarding in C++03.
         //!
         //! For compilers with rvalue references this function provides perfect forwarding.
         //!
         //! Otherwise:
         //! * If input_reference binds to const ::dgboost::rv<T> & then it output_reference is
         //!   ::dgboost::rv<T> &
         //!
         //! * Else, output_reference is equal to input_reference.
         template <class T> output_reference forward(input_reference) noexcept;
      #elif defined(BOOST_MOVE_OLD_RVALUE_REF_BINDING_RULES)

         //Old move approach, lvalues could bind to rvalue references

         template <class T>
         BOOST_MOVE_FORCEINLINE T&& forward(typename ::dgboost::move_detail::identity<T>::type&& t) BOOST_NOEXCEPT
         {  return t;   }

      #else //Old move

         template <class T>
         BOOST_MOVE_FORCEINLINE T&& forward(typename ::dgboost::move_detail::remove_reference<T>::type& t) BOOST_NOEXCEPT
         {  return static_cast<T&&>(t);   }

         template <class T>
         BOOST_MOVE_FORCEINLINE T&& forward(typename ::dgboost::move_detail::remove_reference<T>::type&& t) BOOST_NOEXCEPT
         {
            //"dgboost::forward<T> error: 'T' is a lvalue reference, can't forward as rvalue.";
            BOOST_STATIC_ASSERT(!dgboost::move_detail::is_lvalue_reference<T>::value);
            return static_cast<T&&>(t);
         }

      #endif   //BOOST_MOVE_DOXYGEN_INVOKED

      }  //namespace dgboost {} namespace boost = dgboost; namespace dgboost {

   #endif   //BOOST_MOVE_USE_STANDARD_LIBRARY_MOVE

   //////////////////////////////////////////////////////////////////////////////
   //
   //                         move_if_not_lvalue_reference
   //
   //////////////////////////////////////////////////////////////////////////////

   namespace dgboost {} namespace boost = dgboost; namespace dgboost {

   #if defined(BOOST_MOVE_DOXYGEN_INVOKED)
      //! <b>Effects</b>: Calls `dgboost::move` if `input_reference` is not a lvalue reference.
      //!   Otherwise returns the reference
      template <class T> output_reference move_if_not_lvalue_reference(input_reference) noexcept;
   #elif defined(BOOST_MOVE_OLD_RVALUE_REF_BINDING_RULES)

      //Old move approach, lvalues could bind to rvalue references

      template <class T>
      BOOST_MOVE_FORCEINLINE T&& move_if_not_lvalue_reference(typename ::dgboost::move_detail::identity<T>::type&& t) BOOST_NOEXCEPT
      {  return t;   }

   #else //Old move

      template <class T>
      BOOST_MOVE_FORCEINLINE T&& move_if_not_lvalue_reference(typename ::dgboost::move_detail::remove_reference<T>::type& t) BOOST_NOEXCEPT
      {  return static_cast<T&&>(t);   }

      template <class T>
      BOOST_MOVE_FORCEINLINE T&& move_if_not_lvalue_reference(typename ::dgboost::move_detail::remove_reference<T>::type&& t) BOOST_NOEXCEPT
      {
         //"dgboost::forward<T> error: 'T' is a lvalue reference, can't forward as rvalue.";
         BOOST_STATIC_ASSERT(!dgboost::move_detail::is_lvalue_reference<T>::value);
         return static_cast<T&&>(t);
      }

   #endif   //BOOST_MOVE_DOXYGEN_INVOKED

   }  //namespace dgboost {} namespace boost = dgboost; namespace dgboost {

#endif   //BOOST_NO_CXX11_RVALUE_REFERENCES

#if !defined(BOOST_MOVE_DOXYGEN_INVOKED)

namespace dgboost {} namespace boost = dgboost; namespace dgboost{
namespace move_detail{

template <typename T>
typename dgboost::move_detail::add_rvalue_reference<T>::type declval();

}  //namespace move_detail{
}  //namespace dgboost {} namespace boost = dgboost; namespace dgboost{

#endif   //#if !defined(BOOST_MOVE_DOXYGEN_INVOKED)


#include <dgboost/move/detail/config_end.hpp>

#endif //#ifndef BOOST_MOVE_MOVE_UTILITY_CORE_HPP
