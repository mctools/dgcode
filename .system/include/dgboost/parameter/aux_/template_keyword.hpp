// Copyright Daniel Wallin 2006.
// Copyright Cromwell D. Enage 2017.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_TEMPLATE_KEYWORD_060203_HPP
#define BOOST_PARAMETER_TEMPLATE_KEYWORD_060203_HPP

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    struct template_keyword_base
    {
    };
}}} // namespace dgboost::parameter::aux

#include <dgboost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <type_traits>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    template <typename T>
    using is_template_keyword = ::std::is_base_of<
        ::dgboost::parameter::aux::template_keyword_base
      , typename ::std::remove_const<
            typename ::std::remove_reference<T>::type
        >::type
    >;
}}} // namespace dgboost::parameter::aux

#else   // !defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <dgboost/mpl/bool.hpp>
#include <dgboost/mpl/if.hpp>
#include <dgboost/type_traits/remove_const.hpp>

#if defined(BOOST_PARAMETER_HAS_PERFECT_FORWARDING)
#include <dgboost/type_traits/is_base_of.hpp>
#include <dgboost/type_traits/remove_reference.hpp>
#else
#include <dgboost/type_traits/is_convertible.hpp>
#include <dgboost/type_traits/is_lvalue_reference.hpp>
#endif

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

#if !defined(BOOST_PARAMETER_HAS_PERFECT_FORWARDING)
    template <typename T>
    struct is_template_keyword_aux
      : ::dgboost::mpl::if_<
            ::dgboost::is_convertible<
                T*
              , ::dgboost::parameter::aux::template_keyword_base const*
            >
          , ::dgboost::mpl::true_
          , ::dgboost::mpl::false_
        >::type
    {
    };
#endif  // BOOST_PARAMETER_HAS_PERFECT_FORWARDING

    template <typename T>
    struct is_template_keyword
      : ::dgboost::mpl::if_<
#if defined(BOOST_PARAMETER_HAS_PERFECT_FORWARDING)
            // Cannot use is_convertible<> to check if T is derived from
            // template_keyword_base. -- Cromwell D. Enage
            ::dgboost::is_base_of<
                ::dgboost::parameter::aux::template_keyword_base
              , typename ::dgboost::remove_const<
                    typename ::dgboost::remove_reference<T>::type
                >::type
            >
          , ::dgboost::mpl::true_
          , ::dgboost::mpl::false_
#else
            ::dgboost::is_lvalue_reference<T>
          , ::dgboost::mpl::false_
          , ::dgboost::parameter::aux::is_template_keyword_aux<T>
#endif  // BOOST_PARAMETER_HAS_PERFECT_FORWARDING
        >::type
    {
    };
}}} // namespace dgboost::parameter::aux

#endif  // BOOST_PARAMETER_CAN_USE_MP11
#endif  // include guard

