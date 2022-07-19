// Copyright Daniel Wallin, David Abrahams 2010.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_IS_MAYBE_050329_HPP
#define BOOST_PARAMETER_IS_MAYBE_050329_HPP

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    struct maybe_base
    {
    };
}}} // namespace dgboost::parameter::aux

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <type_traits>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    template <typename T>
    using is_maybe = ::std::is_base_of<
        ::dgboost::parameter::aux::maybe_base
      , typename ::std::remove_const<T>::type
    >;
}}} // namespace dgboost::parameter::aux

#else   // !defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <dgboost/mpl/bool.hpp>
#include <dgboost/mpl/if.hpp>
#include <dgboost/type_traits/is_base_of.hpp>
#include <dgboost/type_traits/remove_const.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    template <typename T>
    struct is_maybe
      : ::dgboost::mpl::if_<
            ::dgboost::is_base_of<
                ::dgboost::parameter::aux::maybe_base
              , typename ::dgboost::remove_const<T>::type
            >
          , ::dgboost::mpl::true_
          , ::dgboost::mpl::false_
        >::type
    {
    };
}}} // namespace dgboost::parameter::aux

#endif  // BOOST_PARAMETER_CAN_USE_MP11
#endif  // include guard

