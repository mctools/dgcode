// Copyright David Abrahams 2005.
// Copyright Cromwell D. Enage 2017.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_AUX_TAG_DWA2005610_HPP
#define BOOST_PARAMETER_AUX_TAG_DWA2005610_HPP

#include <dgboost/parameter/aux_/unwrap_cv_reference.hpp>
#include <dgboost/parameter/aux_/tagged_argument.hpp>
#include <dgboost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11) && \
    !BOOST_WORKAROUND(BOOST_MSVC, >= 1910)
// MSVC-14.1+ assigns rvalue references to tagged_argument instances
// instead of tagged_argument_rref instances with this code.
#include <dgboost/mp11/integral.hpp>
#include <dgboost/mp11/utility.hpp>
#include <type_traits>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux { 

    template <typename Keyword, typename Arg>
    struct tag_if_lvalue_reference
    {
        using type = ::dgboost::parameter::aux::tagged_argument_list_of_1<
            ::dgboost::parameter::aux::tagged_argument<
                Keyword
              , typename ::dgboost::parameter::aux
                ::unwrap_cv_reference<Arg>::type
            >
        >;
    };

    template <typename Keyword, typename Arg>
    struct tag_if_scalar
    {
        using type = ::dgboost::parameter::aux::tagged_argument_list_of_1<
            ::dgboost::parameter::aux
            ::tagged_argument<Keyword,typename ::std::add_const<Arg>::type>
        >;
    };

    template <typename Keyword, typename Arg>
    using tag_if_otherwise = ::dgboost::mp11::mp_if<
        ::std::is_scalar<typename ::std::remove_const<Arg>::type>
      , ::dgboost::parameter::aux::tag_if_scalar<Keyword,Arg>
      , ::dgboost::mp11::mp_identity<
            ::dgboost::parameter::aux::tagged_argument_list_of_1<
                ::dgboost::parameter::aux::tagged_argument_rref<Keyword,Arg>
            >
        >
    >;

    template <typename Keyword, typename Arg>
    using tag = ::dgboost::mp11::mp_if<
        ::dgboost::mp11::mp_if<
            ::std::is_lvalue_reference<Arg>
          , ::dgboost::mp11::mp_true
          , ::dgboost::parameter::aux::is_cv_reference_wrapper<Arg>
        >
      , ::dgboost::parameter::aux::tag_if_lvalue_reference<Keyword,Arg>
      , ::dgboost::parameter::aux::tag_if_otherwise<Keyword,Arg>
    >;
}}} // namespace dgboost::parameter::aux_

#elif defined(BOOST_PARAMETER_HAS_PERFECT_FORWARDING)
#include <dgboost/mpl/bool.hpp>
#include <dgboost/mpl/if.hpp>
#include <dgboost/mpl/eval_if.hpp>
#include <dgboost/mpl/identity.hpp>
#include <dgboost/type_traits/add_const.hpp>
#include <dgboost/type_traits/is_scalar.hpp>
#include <dgboost/type_traits/is_lvalue_reference.hpp>
#include <dgboost/type_traits/remove_const.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux { 

    template <typename Keyword, typename ActualArg>
    struct tag
    {
        typedef typename ::dgboost::parameter::aux
        ::unwrap_cv_reference<ActualArg>::type Arg;
        typedef typename ::dgboost::add_const<Arg>::type ConstArg;
        typedef typename ::dgboost::remove_const<Arg>::type MutArg;
        typedef typename ::dgboost::mpl::eval_if<
            typename ::dgboost::mpl::if_<
                ::dgboost::is_lvalue_reference<ActualArg>
              , ::dgboost::mpl::true_
              , ::dgboost::parameter::aux::is_cv_reference_wrapper<ActualArg>
            >::type
          , ::dgboost::mpl::identity<
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
                ::dgboost::parameter::aux::tagged_argument_list_of_1<
#endif
                    ::dgboost::parameter::aux::tagged_argument<Keyword,Arg>
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
                >
#endif
            >
          , ::dgboost::mpl::if_<
                ::dgboost::is_scalar<MutArg>
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
              , ::dgboost::parameter::aux::tagged_argument_list_of_1<
                    ::dgboost::parameter::aux::tagged_argument<Keyword,ConstArg>
                >
              , ::dgboost::parameter::aux::tagged_argument_list_of_1<
                    ::dgboost::parameter::aux::tagged_argument_rref<Keyword,Arg>
                >
#else
              , ::dgboost::parameter::aux::tagged_argument<Keyword,ConstArg>
              , ::dgboost::parameter::aux::tagged_argument_rref<Keyword,Arg>
#endif
            >
        >::type type;
    };
}}} // namespace dgboost::parameter::aux_

#else   // !defined(BOOST_PARAMETER_HAS_PERFECT_FORWARDING)

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux { 

    template <
        typename Keyword
      , typename Arg
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))
      , typename = typename ::dgboost::parameter::aux
        ::is_cv_reference_wrapper<Arg>::type
#endif
    >
    struct tag
    {
        typedef ::dgboost::parameter::aux::tagged_argument<
            Keyword
          , typename ::dgboost::parameter::aux::unwrap_cv_reference<Arg>::type
        > type;
    };
}}} // namespace dgboost::parameter::aux_

#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x564))
#include <dgboost/mpl/bool.hpp>
#include <dgboost/type_traits/remove_reference.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux { 

    template <typename Keyword, typename Arg>
    struct tag<Keyword,Arg,::dgboost::mpl::false_>
    {
        typedef ::dgboost::parameter::aux::tagged_argument<
            Keyword
          , typename ::dgboost::remove_reference<Arg>::type
        > type;
    };
}}} // namespace dgboost::parameter::aux_

#endif  // Borland workarounds needed.
#endif  // MP11 or perfect forwarding support
#endif  // include guard

