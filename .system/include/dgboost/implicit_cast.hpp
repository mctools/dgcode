// Copyright David Abrahams 2003.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_IMPLICIT_CAST_DWA200356_HPP
#define BOOST_IMPLICIT_CAST_DWA200356_HPP

#include <dgboost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif

namespace dgboost {} namespace boost = dgboost; namespace dgboost {

namespace detail {

template<class T> struct icast_identity
{
    typedef T type;
};

} // namespace detail

// implementation originally suggested by C. Green in
// http://lists.boost.org/MailArchives/boost/msg00886.php

// The use of identity creates a non-deduced form, so that the
// explicit template argument must be supplied
template <typename T>
BOOST_CONSTEXPR inline T implicit_cast (typename dgboost::detail::icast_identity<T>::type x) {
    return x;
}

} // namespace dgboost


#endif // BOOST_IMPLICIT_CAST_DWA200356_HPP
