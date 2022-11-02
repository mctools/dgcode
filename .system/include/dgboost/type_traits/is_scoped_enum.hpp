/*
Copyright 2020 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License,
Version 1.0. (See accompanying file LICENSE_1_0.txt
or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BOOST_TT_IS_SCOPED_ENUM_HPP_INCLUDED
#define BOOST_TT_IS_SCOPED_ENUM_HPP_INCLUDED

#include <dgboost/type_traits/conjunction.hpp>
#include <dgboost/type_traits/is_enum.hpp>
#include <dgboost/type_traits/is_convertible.hpp>
#include <dgboost/type_traits/negation.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost {

template<class T>
struct is_scoped_enum
    : conjunction<is_enum<T>, negation<is_convertible<T, int> > >::type { };

} /* boost */

#endif
