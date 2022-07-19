
#ifndef BOOST_MPL_SET_AUX_HAS_KEY_IMPL_HPP_INCLUDED
#define BOOST_MPL_SET_AUX_HAS_KEY_IMPL_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2003-2004
// Copyright David Abrahams 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id$
// $Date$
// $Revision$

#include <dgboost/mpl/set/aux_/tag.hpp>
#include <dgboost/mpl/has_key_fwd.hpp>
#include <dgboost/mpl/bool.hpp>
#include <dgboost/mpl/aux_/overload_names.hpp>
#include <dgboost/mpl/aux_/static_cast.hpp>
#include <dgboost/mpl/aux_/yes_no.hpp>
#include <dgboost/mpl/aux_/type_wrapper.hpp>
#include <dgboost/mpl/aux_/config/workaround.hpp>
#include <dgboost/mpl/aux_/config/static_constant.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace mpl {

template<>
struct has_key_impl< aux::set_tag >
{
    template< typename Set, typename T > struct apply
#if BOOST_WORKAROUND(BOOST_MSVC, BOOST_TESTED_AT(1400)) \
    || BOOST_WORKAROUND(__EDG_VERSION__, <= 245)
    {
        BOOST_STATIC_CONSTANT(bool, value = 
              ( sizeof( BOOST_MPL_AUX_OVERLOAD_CALL_IS_MASKED(
                    Set
                  , BOOST_MPL_AUX_STATIC_CAST(aux::type_wrapper<T>*, 0)
                  ) ) == sizeof(aux::no_tag) )
            );

        typedef bool_<value> type;

#else // ISO98 C++
        : bool_< 
              ( sizeof( BOOST_MPL_AUX_OVERLOAD_CALL_IS_MASKED(
                    Set
                  , BOOST_MPL_AUX_STATIC_CAST(aux::type_wrapper<T>*, 0)
                  ) ) == sizeof(aux::no_tag) )
            >
    {
#endif
    };
};

}}

#endif // BOOST_MPL_SET_AUX_HAS_KEY_IMPL_HPP_INCLUDED
