//////////////////////////////////////////////////////////////////////////////
// (C) Copyright John Maddock 2000.
// (C) Copyright Ion Gaztanaga 2005-2015.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
// The alignment and Type traits implementation comes from
// John Maddock's TypeTraits library.
//
// Some other tricks come from Howard Hinnant's papers and StackOverflow replies
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_CONTAINER_DETAIL_TYPE_TRAITS_HPP
#define BOOST_CONTAINER_CONTAINER_DETAIL_TYPE_TRAITS_HPP

#ifndef BOOST_CONFIG_HPP
#  include <dgboost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <dgboost/move/detail/type_traits.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost {
namespace container {
namespace dtl {

using ::dgboost::move_detail::enable_if;
using ::dgboost::move_detail::enable_if_and;
using ::dgboost::move_detail::is_same;
using ::dgboost::move_detail::is_different;
using ::dgboost::move_detail::is_pointer;
using ::dgboost::move_detail::add_reference;
using ::dgboost::move_detail::add_const;
using ::dgboost::move_detail::add_const_reference;
using ::dgboost::move_detail::remove_const;
using ::dgboost::move_detail::remove_reference;
using ::dgboost::move_detail::make_unsigned;
using ::dgboost::move_detail::is_floating_point;
using ::dgboost::move_detail::is_integral;
using ::dgboost::move_detail::is_enum;
using ::dgboost::move_detail::is_pod;
using ::dgboost::move_detail::is_empty;
using ::dgboost::move_detail::is_trivially_destructible;
using ::dgboost::move_detail::is_trivially_default_constructible;
using ::dgboost::move_detail::is_trivially_copy_constructible;
using ::dgboost::move_detail::is_trivially_move_constructible;
using ::dgboost::move_detail::is_trivially_copy_assignable;
using ::dgboost::move_detail::is_trivially_move_assignable;
using ::dgboost::move_detail::is_nothrow_default_constructible;
using ::dgboost::move_detail::is_nothrow_copy_constructible;
using ::dgboost::move_detail::is_nothrow_move_constructible;
using ::dgboost::move_detail::is_nothrow_copy_assignable;
using ::dgboost::move_detail::is_nothrow_move_assignable;
using ::dgboost::move_detail::is_nothrow_swappable;
using ::dgboost::move_detail::alignment_of;
using ::dgboost::move_detail::aligned_storage;
using ::dgboost::move_detail::nat;
using ::dgboost::move_detail::nat2;
using ::dgboost::move_detail::nat3;
using ::dgboost::move_detail::max_align_t;

}  //namespace dtl {
}  //namespace container {
}  //namespace dgboost {} namespace boost = dgboost; namespace dgboost {

#endif   //#ifndef BOOST_CONTAINER_CONTAINER_DETAIL_TYPE_TRAITS_HPP
