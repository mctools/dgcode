// Copyright David Abrahams 2005.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_BINDING_DWA200558_HPP
#define BOOST_PARAMETER_BINDING_DWA200558_HPP

#include <dgboost/parameter/aux_/void.hpp>
#include <dgboost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <dgboost/mp11/integral.hpp>
#include <dgboost/mp11/list.hpp>
#include <dgboost/mp11/utility.hpp>
#include <type_traits>
#else
#include <dgboost/mpl/bool.hpp>
#include <dgboost/mpl/if.hpp>
#include <dgboost/mpl/eval_if.hpp>
#include <dgboost/mpl/identity.hpp>
#include <dgboost/mpl/apply_wrap.hpp>
#include <dgboost/mpl/assert.hpp>
#include <dgboost/type_traits/is_same.hpp>
#endif

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { 

    // A metafunction that, given an argument pack, returns the reference type
    // of the parameter identified by the given keyword.  If no such parameter
    // has been specified, returns Default

    template <typename Parameters, typename Keyword, typename Default>
    struct binding0
    {
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
        using type = ::dgboost::mp11::mp_apply_q<
            typename Parameters::binding
          , ::dgboost::mp11::mp_list<Keyword,Default,::dgboost::mp11::mp_true>
        >;

        static_assert(
            ::dgboost::mp11::mp_if<
                ::std::is_same<Default,::dgboost::parameter::void_>
              , ::dgboost::mp11::mp_if<
                    ::std::is_same<type,::dgboost::parameter::void_>
                  , ::dgboost::mp11::mp_false
                  , ::dgboost::mp11::mp_true
                >
              , ::dgboost::mp11::mp_true
            >::value
          , "required parameters must not result in void_ type"
        );
#else   // !defined(BOOST_PARAMETER_CAN_USE_MP11)
        typedef typename ::dgboost::mpl::apply_wrap3<
            typename Parameters::binding
          , Keyword
          , Default
          , ::dgboost::mpl::true_
        >::type type;

        BOOST_MPL_ASSERT((
            typename ::dgboost::mpl::eval_if<
                ::dgboost::is_same<Default,::dgboost::parameter::void_>
              , ::dgboost::mpl::if_<
                    ::dgboost::is_same<type,::dgboost::parameter::void_>
                  , ::dgboost::mpl::false_
                  , ::dgboost::mpl::true_
                >
              , ::dgboost::mpl::true_
            >::type
        ));
#endif  // BOOST_PARAMETER_CAN_USE_MP11
    };

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
    template <typename Placeholder, typename Keyword, typename Default>
    struct binding1
    {
        using type = ::dgboost::mp11::mp_apply_q<
            Placeholder
          , ::dgboost::mp11::mp_list<Keyword,Default,::dgboost::mp11::mp_true>
        >;

        static_assert(
            ::dgboost::mp11::mp_if<
                ::std::is_same<Default,::dgboost::parameter::void_>
              , ::dgboost::mp11::mp_if<
                    ::std::is_same<type,::dgboost::parameter::void_>
                  , ::dgboost::mp11::mp_false
                  , ::dgboost::mp11::mp_true
                >
              , ::dgboost::mp11::mp_true
            >::value
          , "required parameters must not result in void_ type"
        );
    };
#endif  // BOOST_PARAMETER_CAN_USE_MP11
}} // namespace dgboost::parameter

#include <dgboost/parameter/aux_/is_placeholder.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { 

    template <
        typename Parameters
      , typename Keyword
      , typename Default = ::dgboost::parameter::void_
    >
    struct binding
#if !defined(BOOST_PARAMETER_CAN_USE_MP11)
      : ::dgboost::mpl::eval_if<
            ::dgboost::parameter::aux::is_mpl_placeholder<Parameters>
          , ::dgboost::mpl::identity<int>
          , ::dgboost::parameter::binding0<Parameters,Keyword,Default>
        >
#endif
    {
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
        using type = typename ::dgboost::mp11::mp_if<
            ::dgboost::parameter::aux::is_mpl_placeholder<Parameters>
          , ::dgboost::mp11::mp_identity<int>
          , ::dgboost::mp11::mp_if<
                ::dgboost::parameter::aux::is_mp11_placeholder<Parameters>
              , ::dgboost::parameter::binding1<Parameters,Keyword,Default>
              , ::dgboost::parameter::binding0<Parameters,Keyword,Default>
            >
        >::type;
#endif
    };
}} // namespace dgboost::parameter

#include <dgboost/parameter/aux_/result_of0.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { 

    // A metafunction that, given an argument pack, returns the reference type
    // of the parameter identified by the given keyword.  If no such parameter
    // has been specified, returns the type returned by invoking DefaultFn
    template <typename Parameters, typename Keyword, typename DefaultFn>
    struct lazy_binding
    {
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
        using type = ::dgboost::mp11::mp_apply_q<
            typename Parameters::binding
          , ::dgboost::mp11::mp_list<
                Keyword
              , typename ::dgboost::parameter::aux::result_of0<DefaultFn>::type
              , ::dgboost::mp11::mp_true
            >
        >;
#else
        typedef typename ::dgboost::mpl::apply_wrap3<
            typename Parameters::binding
          , Keyword
          , typename ::dgboost::parameter::aux::result_of0<DefaultFn>::type
          , ::dgboost::mpl::true_
        >::type type;
#endif  // BOOST_PARAMETER_CAN_USE_MP11
    };
}} // namespace dgboost::parameter

#endif  // include guard

