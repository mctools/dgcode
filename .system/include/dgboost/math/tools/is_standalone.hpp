//  Copyright Matt Borland 2021.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_MATH_TOOLS_IS_STANDALONE_HPP
#define BOOST_MATH_TOOLS_IS_STANDALONE_HPP

#ifdef __has_include
#if !__has_include(<dgboost/config.hpp>) || !__has_include(<dgboost/assert.hpp>) || !__has_include(<dgboost/lexical_cast.hpp>) || \
    !__has_include(<dgboost/throw_exception.hpp>) || !__has_include(<dgboost/predef/other/endian.h>)
#   ifndef BOOST_MATH_STANDALONE
#       define BOOST_MATH_STANDALONE
#   endif
#endif
#endif

#endif // BOOST_MATH_TOOLS_IS_STANDALONE_HPP
