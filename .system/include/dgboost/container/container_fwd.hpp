//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2014. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_CONTAINER_FWD_HPP
#define BOOST_CONTAINER_CONTAINER_FWD_HPP

#ifndef BOOST_CONFIG_HPP
#  include <dgboost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

//! \file
//! This header file forward declares the following containers:
//!   - dgboost::container::vector
//!   - dgboost::container::stable_vector
//!   - dgboost::container::static_vector
//!   - dgboost::container::small_vector_base
//!   - dgboost::container::small_vector
//!   - dgboost::container::devector
//!   - dgboost::container::slist
//!   - dgboost::container::list
//!   - dgboost::container::set
//!   - dgboost::container::multiset
//!   - dgboost::container::map
//!   - dgboost::container::multimap
//!   - dgboost::container::flat_set
//!   - dgboost::container::flat_multiset
//!   - dgboost::container::flat_map
//!   - dgboost::container::flat_multimap
//!   - dgboost::container::basic_string
//!   - dgboost::container::string
//!   - dgboost::container::wstring
//!
//! Forward declares the following allocators:
//!   - dgboost::container::allocator
//!   - dgboost::container::node_allocator
//!   - dgboost::container::adaptive_pool
//!
//! Forward declares the following polymorphic resource classes:
//!   - dgboost::container::pmr::memory_resource
//!   - dgboost::container::pmr::polymorphic_allocator
//!   - dgboost::container::pmr::monotonic_buffer_resource
//!   - dgboost::container::pmr::pool_options
//!   - dgboost::container::pmr::unsynchronized_pool_resource
//!   - dgboost::container::pmr::synchronized_pool_resource
//!
//! And finally it defines the following types

#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

//Std forward declarations
#ifndef BOOST_CONTAINER_DETAIL_STD_FWD_HPP
   #include <dgboost/container/detail/std_fwd.hpp>
#endif

namespace dgboost {} namespace boost = dgboost; namespace dgboost{
namespace intrusive{
namespace detail{
   //Create namespace to avoid compilation errors
}}}

namespace dgboost {} namespace boost = dgboost; namespace dgboost{ namespace container{ namespace dtl{
   namespace bi = dgboost::intrusive;
   namespace bid = dgboost::intrusive::detail;
}}}

namespace dgboost {} namespace boost = dgboost; namespace dgboost{ namespace container{ namespace pmr{
   namespace bi = dgboost::intrusive;
   namespace bid = dgboost::intrusive::detail;
}}}

#include <cstddef>

#endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

//////////////////////////////////////////////////////////////////////////////
//                             Containers
//////////////////////////////////////////////////////////////////////////////

namespace dgboost {} namespace boost = dgboost; namespace dgboost {
namespace container {

#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

template<class T>
class new_allocator;

template <class T
         ,class Allocator = void
         ,class Options   = void>
class vector;

template <class T
         ,class Allocator = void >
class stable_vector;

template < class T
         , std::size_t Capacity
         , class Options = void>
class static_vector;

template < class T
         , class Allocator = void
         , class Options   = void >
class small_vector_base;

template < class T
         , std::size_t N
         , class Allocator = void
         , class Options   = void  >
class small_vector;

template <class T
         ,class Allocator = void
         ,class Options   = void>
class devector;

template <class T
         ,class Allocator = void
         ,class Options   = void>
class deque;

template <class T
         ,class Allocator = void >
class list;

template <class T
         ,class Allocator = void >
class slist;

template <class Key
         ,class Compare  = std::less<Key>
         ,class Allocator = void
         ,class Options = void>
class set;

template <class Key
         ,class Compare  = std::less<Key>
         ,class Allocator = void
         ,class Options = void >
class multiset;

template <class Key
         ,class T
         ,class Compare  = std::less<Key>
         ,class Allocator = void
         ,class Options = void >
class map;

template <class Key
         ,class T
         ,class Compare  = std::less<Key>
         ,class Allocator = void
         ,class Options = void >
class multimap;

template <class Key
         ,class Compare  = std::less<Key>
         ,class Allocator = void >
class flat_set;

template <class Key
         ,class Compare  = std::less<Key>
         ,class Allocator = void >
class flat_multiset;

template <class Key
         ,class T
         ,class Compare  = std::less<Key>
         ,class Allocator = void >
class flat_map;

template <class Key
         ,class T
         ,class Compare  = std::less<Key>
         ,class Allocator = void >
class flat_multimap;

#ifndef BOOST_NO_CXX11_TEMPLATE_ALIASES

//! Alias templates for small_flat_[multi]{set|map} using small_vector as container

template < class Key
         , std::size_t N
         , class Compare  = std::less<Key>
         , class SmallVectorAllocator = void
         , class SmallVectorOptions   = void  >
using small_flat_set = flat_set<Key, Compare, small_vector<Key, N, SmallVectorAllocator, SmallVectorOptions>>;

template < class Key
         , std::size_t N
         , class Compare  = std::less<Key>
         , class SmallVectorAllocator = void
         , class SmallVectorOptions   = void  >
using small_flat_multiset = flat_multiset<Key, Compare, small_vector<Key, N, SmallVectorAllocator, SmallVectorOptions>>;

template < class Key
         , class T
         , std::size_t N
         , class Compare  = std::less<Key>
         , class SmallVectorAllocator = void
         , class SmallVectorOptions   = void  >
using small_flat_map = flat_map<Key, T, Compare, small_vector<std::pair<Key, T>, N, SmallVectorAllocator, SmallVectorOptions>>;

template < class Key
         , class T
         , std::size_t N
         , class Compare  = std::less<Key>
         , class SmallVectorAllocator = void
         , class SmallVectorOptions   = void  >
using small_flat_multimap = flat_multimap<Key, T, Compare, small_vector<std::pair<Key, T>, N, SmallVectorAllocator, SmallVectorOptions>>;

#endif // #ifndef BOOST_NO_CXX11_TEMPLATE_ALIASES


//! A portable metafunction to obtain a small_flat_set
template < class Key
         , std::size_t N
         , class Compare  = std::less<Key>
         , class SmallVectorAllocator = void
         , class SmallVectorOptions   = void  >
struct small_flat_set_of
{
   typedef flat_set<Key, Compare, small_vector<Key, N, SmallVectorAllocator, SmallVectorOptions> > type;
};

//! A portable metafunction to obtain a small_flat_multiset
template < class Key
         , std::size_t N
         , class Compare  = std::less<Key>
         , class SmallVectorAllocator = void
         , class SmallVectorOptions   = void  >
struct small_flat_multiset_of
{
   typedef flat_multiset<Key, Compare, small_vector<Key, N, SmallVectorAllocator, SmallVectorOptions> > type;
};

//! A portable metafunction to obtain a small_flat_map
template < class Key
         , class T
         , std::size_t N
         , class Compare  = std::less<Key>
         , class SmallVectorAllocator = void
         , class SmallVectorOptions   = void  >
struct small_flat_map_of
{
   typedef flat_map<Key, T, Compare, small_vector<std::pair<Key, T>, N, SmallVectorAllocator, SmallVectorOptions> > type;
};

//! A portable metafunction to obtain a small_flat_multimap
template < class Key
         , class T
         , std::size_t N
         , class Compare  = std::less<Key>
         , class SmallVectorAllocator = void
         , class SmallVectorOptions   = void  >
struct small_flat_multimap_of
{
   typedef flat_multimap<Key, T, Compare, small_vector<std::pair<Key, T>, N, SmallVectorAllocator, SmallVectorOptions> > type;
};

template <class CharT
         ,class Traits = std::char_traits<CharT>
         ,class Allocator  = void >
class basic_string;

typedef basic_string <char>   string;
typedef basic_string<wchar_t> wstring;

static const std::size_t ADP_nodes_per_block    = 256u;
static const std::size_t ADP_max_free_blocks    = 2u;
static const std::size_t ADP_overhead_percent   = 1u;
static const std::size_t ADP_only_alignment     = 0u;

template < class T
         , std::size_t NodesPerBlock   = ADP_nodes_per_block
         , std::size_t MaxFreeBlocks   = ADP_max_free_blocks
         , std::size_t OverheadPercent = ADP_overhead_percent
         , unsigned Version = 2
         >
class adaptive_pool;

template < class T
         , unsigned Version = 2
         , unsigned int AllocationDisableMask = 0>
class allocator;

static const std::size_t NodeAlloc_nodes_per_block = 256u;

template
   < class T
   , std::size_t NodesPerBlock = NodeAlloc_nodes_per_block
   , std::size_t Version = 2>
class node_allocator;

namespace pmr {

class memory_resource;

template<class T>
class polymorphic_allocator;

class monotonic_buffer_resource;

struct pool_options;

template <class Allocator>
class resource_adaptor_imp;

class unsynchronized_pool_resource;

class synchronized_pool_resource;

}  //namespace pmr {

#endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

//! Type used to tag that the input range is
//! guaranteed to be ordered
struct ordered_range_t
{};

//! Value used to tag that the input range is
//! guaranteed to be ordered
static const ordered_range_t ordered_range = ordered_range_t();

//! Type used to tag that the input range is
//! guaranteed to be ordered and unique
struct ordered_unique_range_t
   : public ordered_range_t
{};

//! Value used to tag that the input range is
//! guaranteed to be ordered and unique
static const ordered_unique_range_t ordered_unique_range = ordered_unique_range_t();

//! Type used to tag that the inserted values
//! should be default initialized
struct default_init_t
{};

//! Value used to tag that the inserted values
//! should be default initialized
static const default_init_t default_init = default_init_t();
#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

//! Type used to tag that the inserted values
//! should be value initialized
struct value_init_t
{};

//! Value used to tag that the inserted values
//! should be value initialized
static const value_init_t value_init = value_init_t();

namespace container_detail_really_deep_namespace {

//Otherwise, gcc issues a warning of previously defined
//anonymous_instance and unique_instance
struct dummy
{
   dummy()
   {
      (void)ordered_range;
      (void)ordered_unique_range;
      (void)default_init;
   }
};

}  //detail_really_deep_namespace {

typedef const std::piecewise_construct_t & piecewise_construct_t;

#endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

}}  //namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace container {

#endif //#ifndef BOOST_CONTAINER_CONTAINER_FWD_HPP
