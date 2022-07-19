// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef BOOST_RANGE_VALUE_TYPE_HPP
#define BOOST_RANGE_VALUE_TYPE_HPP

#if defined(_MSC_VER)
# pragma once
#endif

#include <dgboost/range/config.hpp>
#include <dgboost/range/iterator.hpp>

#include <dgboost/iterator/iterator_traits.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost
{
    template< class T >
    struct range_value : iterator_value< typename range_iterator<T>::type >
    { };
}

#endif
