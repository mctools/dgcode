// Copyright Cromwell D. Enage 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_IS_ARGUMENT_PACK_HPP
#define BOOST_PARAMETER_IS_ARGUMENT_PACK_HPP

#include <dgboost/parameter/aux_/is_tagged_argument.hpp>
#include <dgboost/parameter/aux_/arg_list.hpp>
#include <dgboost/mpl/bool.hpp>
#include <dgboost/mpl/if.hpp>
#include <dgboost/type_traits/is_base_of.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter {

    template <typename T>
    struct is_argument_pack
      : ::dgboost::mpl::if_<
            ::dgboost::is_base_of< ::dgboost::parameter::aux::empty_arg_list,T>
          , ::dgboost::mpl::true_
          , ::dgboost::parameter::aux::is_tagged_argument<T>
        >::type
    {
    };
}}

#endif  // include guard

