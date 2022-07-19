// Copyright Cromwell D. Enage 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_AUGMENT_PREDICATE_HPP
#define BOOST_PARAMETER_AUGMENT_PREDICATE_HPP

#include <dgboost/parameter/keyword_fwd.hpp>
#include <dgboost/mpl/bool.hpp>
#include <dgboost/mpl/if.hpp>
#include <dgboost/mpl/eval_if.hpp>
#include <dgboost/type_traits/is_lvalue_reference.hpp>
#include <dgboost/type_traits/is_scalar.hpp>
#include <dgboost/type_traits/is_same.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    template <typename V, typename R, typename Tag>
    struct augment_predicate_check_consume_ref
      : ::dgboost::mpl::eval_if<
            ::dgboost::is_scalar<V>
          , ::dgboost::mpl::true_
          , ::dgboost::mpl::eval_if<
                ::dgboost::is_same<
                    typename Tag::qualifier
                  , ::dgboost::parameter::consume_reference
                >
              , ::dgboost::mpl::if_<
                    ::dgboost::is_lvalue_reference<R>
                  , ::dgboost::mpl::false_
                  , ::dgboost::mpl::true_
                >
              , dgboost::mpl::true_
            >
        >::type
    {
    };
}}} // namespace dgboost::parameter::aux

#include <dgboost/type_traits/is_const.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    template <typename V, typename R, typename Tag>
    struct augment_predicate_check_out_ref
      : ::dgboost::mpl::eval_if<
            ::dgboost::is_same<
                typename Tag::qualifier
              , ::dgboost::parameter::out_reference
            >
          , ::dgboost::mpl::eval_if<
                ::dgboost::is_lvalue_reference<R>
              , ::dgboost::mpl::if_<
                    ::dgboost::is_const<V>
                  , ::dgboost::mpl::false_
                  , ::dgboost::mpl::true_
                >
              , ::dgboost::mpl::false_
            >
          , ::dgboost::mpl::true_
        >::type
    {
    };
}}} // namespace dgboost::parameter::aux

#include <dgboost/parameter/aux_/lambda_tag.hpp>
#include <dgboost/mpl/apply_wrap.hpp>
#include <dgboost/mpl/lambda.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    template <
        typename Predicate
      , typename R
      , typename Tag
      , typename T
      , typename Args
    >
    class augment_predicate
    {
        typedef typename ::dgboost::mpl::lambda<
            Predicate
          , ::dgboost::parameter::aux::lambda_tag
        >::type _actual_predicate;

     public:
        typedef typename ::dgboost::mpl::eval_if<
            typename ::dgboost::mpl::if_<
                ::dgboost::parameter::aux
                ::augment_predicate_check_consume_ref<T,R,Tag>
              , ::dgboost::parameter::aux
                ::augment_predicate_check_out_ref<T,R,Tag>
              , ::dgboost::mpl::false_
            >::type
          , ::dgboost::mpl::apply_wrap2<_actual_predicate,T,Args>
          , ::dgboost::mpl::false_
        >::type type;
    };
}}} // namespace dgboost::parameter::aux

#include <dgboost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <dgboost/mp11/integral.hpp>
#include <dgboost/mp11/utility.hpp>
#include <type_traits>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    template <typename V, typename R, typename Tag>
    using augment_predicate_check_consume_ref_mp11 = ::dgboost::mp11::mp_if<
        ::std::is_scalar<V>
      , ::dgboost::mp11::mp_true
      , ::dgboost::mp11::mp_if<
            ::std::is_same<
                typename Tag::qualifier
              , ::dgboost::parameter::consume_reference
            >
          , ::dgboost::mp11::mp_if<
                ::std::is_lvalue_reference<R>
              , ::dgboost::mp11::mp_false
              , ::dgboost::mp11::mp_true
            >
          , dgboost::mp11::mp_true
        >
    >;

    template <typename V, typename R, typename Tag>
    using augment_predicate_check_out_ref_mp11 = ::dgboost::mp11::mp_if<
        ::std::is_same<
            typename Tag::qualifier
          , ::dgboost::parameter::out_reference
        >
      , ::dgboost::mp11::mp_if<
            ::std::is_lvalue_reference<R>
          , ::dgboost::mp11::mp_if<
                ::std::is_const<V>
              , ::dgboost::mp11::mp_false
              , ::dgboost::mp11::mp_true
            >
          , ::dgboost::mp11::mp_false
        >
      , ::dgboost::mp11::mp_true
    >;
}}} // namespace dgboost::parameter::aux

#include <dgboost/mp11/list.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    template <
        typename Predicate
      , typename R
      , typename Tag
      , typename T
      , typename Args
    >
    struct augment_predicate_mp11_impl
    {
        using type = ::dgboost::mp11::mp_if<
            ::dgboost::mp11::mp_if<
                ::dgboost::parameter::aux
                ::augment_predicate_check_consume_ref_mp11<T,R,Tag>
              , ::dgboost::parameter::aux
                ::augment_predicate_check_out_ref_mp11<T,R,Tag>
              , ::dgboost::mp11::mp_false
            >
          , ::dgboost::mp11
            ::mp_apply_q<Predicate,::dgboost::mp11::mp_list<T,Args> >
          , ::dgboost::mp11::mp_false
        >;
    };
}}} // namespace dgboost::parameter::aux

#include <dgboost/parameter/aux_/has_nested_template_fn.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    template <
        typename Predicate
      , typename R
      , typename Tag
      , typename T
      , typename Args
    >
    using augment_predicate_mp11 = ::dgboost::mp11::mp_if<
        ::dgboost::parameter::aux::has_nested_template_fn<Predicate>
      , ::dgboost::parameter::aux
        ::augment_predicate_mp11_impl<Predicate,R,Tag,T,Args>
      , ::dgboost::parameter::aux
        ::augment_predicate<Predicate,R,Tag,T,Args>
    >;
}}} // namespace dgboost::parameter::aux

#endif  // BOOST_PARAMETER_CAN_USE_MP11
#endif  // include guard

