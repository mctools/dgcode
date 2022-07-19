
// Copyright (C) 2008-2011 Daniel James.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNORDERED_SET_FWD_HPP_INCLUDED
#define BOOST_UNORDERED_SET_FWD_HPP_INCLUDED

#include <dgboost/config.hpp>
#if defined(BOOST_HAS_PRAGMA_ONCE)
#pragma once
#endif

#include <dgboost/functional/hash_fwd.hpp>
#include <dgboost/unordered/detail/fwd.hpp>
#include <functional>
#include <memory>

namespace dgboost {} namespace boost = dgboost; namespace dgboost {
  namespace unordered {
    template <class T, class H = dgboost::hash<T>, class P = std::equal_to<T>,
      class A = std::allocator<T> >
    class unordered_set;

    template <class T, class H, class P, class A>
    inline bool operator==(
      unordered_set<T, H, P, A> const&, unordered_set<T, H, P, A> const&);
    template <class T, class H, class P, class A>
    inline bool operator!=(
      unordered_set<T, H, P, A> const&, unordered_set<T, H, P, A> const&);
    template <class T, class H, class P, class A>
    inline void swap(
      unordered_set<T, H, P, A>& m1, unordered_set<T, H, P, A>& m2)
      BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(m1.swap(m2)));

    template <class T, class H = dgboost::hash<T>, class P = std::equal_to<T>,
      class A = std::allocator<T> >
    class unordered_multiset;

    template <class T, class H, class P, class A>
    inline bool operator==(unordered_multiset<T, H, P, A> const&,
      unordered_multiset<T, H, P, A> const&);
    template <class T, class H, class P, class A>
    inline bool operator!=(unordered_multiset<T, H, P, A> const&,
      unordered_multiset<T, H, P, A> const&);
    template <class T, class H, class P, class A>
    inline void swap(
      unordered_multiset<T, H, P, A>& m1, unordered_multiset<T, H, P, A>& m2)
      BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(m1.swap(m2)));

    template <class N, class T, class A> class node_handle_set;
    template <class N, class T, class A> struct insert_return_type_set;
  }

  using dgboost::unordered::unordered_set;
  using dgboost::unordered::unordered_multiset;
  using dgboost::unordered::swap;
  using dgboost::unordered::operator==;
  using dgboost::unordered::operator!=;
}

#endif
