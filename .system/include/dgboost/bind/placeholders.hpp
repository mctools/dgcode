#ifndef BOOST_BIND_PLACEHOLDERS_HPP_INCLUDED
#define BOOST_BIND_PLACEHOLDERS_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//
//  bind/placeholders.hpp - _N definitions
//
//  Copyright (c) 2002 Peter Dimov and Multi Media Ltd.
//  Copyright 2015 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//
//  See http://www.boost.org/libs/bind/bind.html for documentation.
//

#include <dgboost/bind/arg.hpp>
#include <dgboost/config.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost
{

namespace placeholders
{

#if defined(BOOST_BORLANDC) || defined(__GNUC__) && (__GNUC__ < 4)

inline dgboost::arg<1> _1() { return dgboost::arg<1>(); }
inline dgboost::arg<2> _2() { return dgboost::arg<2>(); }
inline dgboost::arg<3> _3() { return dgboost::arg<3>(); }
inline dgboost::arg<4> _4() { return dgboost::arg<4>(); }
inline dgboost::arg<5> _5() { return dgboost::arg<5>(); }
inline dgboost::arg<6> _6() { return dgboost::arg<6>(); }
inline dgboost::arg<7> _7() { return dgboost::arg<7>(); }
inline dgboost::arg<8> _8() { return dgboost::arg<8>(); }
inline dgboost::arg<9> _9() { return dgboost::arg<9>(); }

#elif !defined(BOOST_NO_CXX17_INLINE_VARIABLES)

BOOST_INLINE_CONSTEXPR dgboost::arg<1> _1;
BOOST_INLINE_CONSTEXPR dgboost::arg<2> _2;
BOOST_INLINE_CONSTEXPR dgboost::arg<3> _3;
BOOST_INLINE_CONSTEXPR dgboost::arg<4> _4;
BOOST_INLINE_CONSTEXPR dgboost::arg<5> _5;
BOOST_INLINE_CONSTEXPR dgboost::arg<6> _6;
BOOST_INLINE_CONSTEXPR dgboost::arg<7> _7;
BOOST_INLINE_CONSTEXPR dgboost::arg<8> _8;
BOOST_INLINE_CONSTEXPR dgboost::arg<9> _9;

#else

BOOST_STATIC_CONSTEXPR dgboost::arg<1> _1;
BOOST_STATIC_CONSTEXPR dgboost::arg<2> _2;
BOOST_STATIC_CONSTEXPR dgboost::arg<3> _3;
BOOST_STATIC_CONSTEXPR dgboost::arg<4> _4;
BOOST_STATIC_CONSTEXPR dgboost::arg<5> _5;
BOOST_STATIC_CONSTEXPR dgboost::arg<6> _6;
BOOST_STATIC_CONSTEXPR dgboost::arg<7> _7;
BOOST_STATIC_CONSTEXPR dgboost::arg<8> _8;
BOOST_STATIC_CONSTEXPR dgboost::arg<9> _9;

#endif

} // namespace placeholders

} // namespace dgboost

#endif // #ifndef BOOST_BIND_PLACEHOLDERS_HPP_INCLUDED
