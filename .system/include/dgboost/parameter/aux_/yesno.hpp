// Copyright Daniel Wallin, David Abrahams 2005.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef YESNO_050328_HPP
#define YESNO_050328_HPP

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    // types used with the "sizeof trick" to capture the results of
    // overload resolution at compile-time.
    typedef char yes_tag;
    typedef char (&no_tag)[2];
}}} // namespace dgboost::parameter::aux

#include <dgboost/mpl/bool.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    // mpl::true_ and mpl::false_ are not distinguishable by sizeof(),
    // so we pass them through these functions to get a type that is.
    ::dgboost::parameter::aux::yes_tag to_yesno(::dgboost::mpl::true_);
    ::dgboost::parameter::aux::no_tag to_yesno(::dgboost::mpl::false_);
}}} // namespace dgboost::parameter::aux

#include <dgboost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <dgboost/mp11/integral.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    // mp11::mp_true and mp11::mp_false are not distinguishable by sizeof(),
    // so we pass them through these functions to get a type that is.
    ::dgboost::parameter::aux::yes_tag to_yesno(::dgboost::mp11::mp_true);
    ::dgboost::parameter::aux::no_tag to_yesno(::dgboost::mp11::mp_false);
}}} // namespace dgboost::parameter::aux

#endif  // BOOST_PARAMETER_CAN_USE_MP11
#endif  // include guard

