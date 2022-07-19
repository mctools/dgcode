// Copyright Daniel Wallin 2006.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_AUX_NAME_HPP
#define BOOST_PARAMETER_AUX_NAME_HPP

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    struct name_tag_base
    {
    };

    template <typename Tag>
    struct name_tag
    {
    };
}}} // namespace dgboost::parameter::aux

#include <dgboost/mpl/bool.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    template <typename T>
    struct is_name_tag : ::dgboost::mpl::false_
    {
    };
}}} // namespace dgboost::parameter::aux

#include <dgboost/parameter/value_type.hpp>
#include <dgboost/mpl/placeholders.hpp>
#include <dgboost/config.hpp>
#include <dgboost/config/workaround.hpp>

#if !defined(BOOST_NO_SFINAE) && \
    !BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x592))
#include <dgboost/parameter/aux_/lambda_tag.hpp>
#include <dgboost/mpl/lambda.hpp>
#include <dgboost/mpl/bind.hpp>
#include <dgboost/mpl/quote.hpp>
#include <dgboost/core/enable_if.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace mpl {

    template <typename T>
    struct lambda<
        T
      , typename ::dgboost::enable_if<
            ::dgboost::parameter::aux::is_name_tag<T>
          , ::dgboost::parameter::aux::lambda_tag
        >::type
    >
    {
        typedef ::dgboost::mpl::true_ is_le;
        typedef ::dgboost::mpl::bind3<
            ::dgboost::mpl::quote3< ::dgboost::parameter::value_type>
          , ::dgboost::mpl::arg<2>
          , T
          , void
        > result_;
        typedef result_ type;
    };
}} // namespace dgboost::mpl

#endif  // SFINAE enabled, not Borland.

#include <dgboost/parameter/aux_/void.hpp>

#define BOOST_PARAMETER_TAG_PLACEHOLDER_TYPE(tag)                            \
    ::dgboost::parameter::value_type<                                          \
        ::dgboost::mpl::_2,tag,::dgboost::parameter::void_                       \
    >
/**/

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#define BOOST_PARAMETER_TAG_MP11_PLACEHOLDER_VALUE(name, tag)                \
    template <typename ArgumentPack>                                         \
    using name = typename ::dgboost::parameter                                 \
    ::value_type<ArgumentPack,tag,::dgboost::parameter::void_>::type
/**/

#include <dgboost/parameter/binding.hpp>

#define BOOST_PARAMETER_TAG_MP11_PLACEHOLDER_BINDING(name, tag)              \
    template <typename ArgumentPack>                                         \
    using name = typename ::dgboost::parameter                                 \
    ::binding<ArgumentPack,tag,::dgboost::parameter::void_>::type
/**/

#endif  // BOOST_PARAMETER_CAN_USE_MP11
#endif  // include guard

