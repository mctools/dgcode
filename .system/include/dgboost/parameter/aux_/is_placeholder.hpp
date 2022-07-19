// Copyright Cromwell D. Enage 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_AUX_IS_PLACEHOLDER_HPP
#define BOOST_PARAMETER_AUX_IS_PLACEHOLDER_HPP

#include <dgboost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <dgboost/mp11/integral.hpp>
#else
#include <dgboost/mpl/bool.hpp>
#endif

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    template <typename T>
    struct is_mpl_placeholder
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
      : ::dgboost::mp11::mp_false
#else
      : ::dgboost::mpl::false_
#endif
    {
    };
}}} // namespace dgboost::parameter::aux

#include <dgboost/mpl/arg_fwd.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    template <int I>
    struct is_mpl_placeholder< ::dgboost::mpl::arg<I> >
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
      : ::dgboost::mp11::mp_true
#else
      : ::dgboost::mpl::true_
#endif
    {
    };
}}} // namespace dgboost::parameter::aux

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <dgboost/mp11/bind.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    template <typename T>
    struct is_mp11_placeholder : ::dgboost::mp11::mp_false
    {
    };

    template < ::std::size_t I>
    struct is_mp11_placeholder< ::dgboost::mp11::mp_arg<I> >
      : ::dgboost::mp11::mp_true
    {
    };
}}} // namespace dgboost::parameter::aux

#endif  // BOOST_PARAMETER_CAN_USE_MP11
#endif  // include guard

