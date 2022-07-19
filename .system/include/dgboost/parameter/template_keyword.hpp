// Copyright Daniel Wallin 2006.
// Copyright Cromwell D. Enage 2017.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_TEMPLATE_KEYWORD_HPP
#define BOOST_PARAMETER_TEMPLATE_KEYWORD_HPP

#include <dgboost/parameter/aux_/template_keyword.hpp>
#include <dgboost/parameter/config.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <dgboost/mp11/integral.hpp>
#include <dgboost/mp11/utility.hpp>
#include <type_traits>
#else
#include <dgboost/mpl/bool.hpp>
#include <dgboost/mpl/if.hpp>
#include <dgboost/mpl/eval_if.hpp>
#include <dgboost/mpl/identity.hpp>
#include <dgboost/type_traits/add_lvalue_reference.hpp>
#include <dgboost/type_traits/is_function.hpp>
#include <dgboost/type_traits/is_array.hpp>
#endif

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { 

    template <typename Tag, typename T>
    struct template_keyword : ::dgboost::parameter::aux::template_keyword_base
    {
        typedef Tag key_type;
        typedef T value_type;

        // reference is needed for two reasons:
        //
        // 1. It is used in the body of arg_list<...>
        //
        // 2. It is the result of binding<...>, which we mistakenly told
        //    people to use instead of value_type<...> to access named
        //    template parameters
        //
        // It used to be that reference == value_type, but that broke when
        // the argument was a function or array type, because various
        // arg_list functions return reference.
        //
        // Simply making reference == value_type& would break all the
        // legacy code that uses binding<...> to access named template
        // parameters. -- David Abrahams
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
        using reference = typename ::dgboost::mp11::mp_eval_if<
            ::dgboost::mp11::mp_if<
                ::std::is_function<value_type>
              , ::dgboost::mp11::mp_true
              , ::std::is_array<value_type>
            >
          , ::std::add_lvalue_reference<value_type>
          , ::dgboost::mp11::mp_identity
          , value_type
        >::type;
#else
        typedef typename ::dgboost::mpl::eval_if<
            typename ::dgboost::mpl::if_<
                ::dgboost::is_function<value_type>
              , ::dgboost::mpl::true_
              , ::dgboost::is_array<value_type>
            >::type
          , ::dgboost::add_lvalue_reference<value_type>
          , ::dgboost::mpl::identity<value_type>
        >::type reference;
#endif  // BOOST_PARAMETER_CAN_USE_MP11
    };
}} // namespace dgboost::parameter

#define BOOST_PARAMETER_TEMPLATE_KEYWORD(name)                               \
    namespace tag                                                            \
    {                                                                        \
        struct name;                                                         \
    }                                                                        \
    template <typename T>                                                    \
    struct name : ::dgboost::parameter::template_keyword<tag::name,T>          \
    {                                                                        \
    };
/**/

#endif  // include guard

