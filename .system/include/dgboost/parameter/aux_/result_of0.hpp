// Copyright David Abrahams 2005.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PARAMETER_AUX_RESULT_OF0_DWA2005511_HPP
#define BOOST_PARAMETER_AUX_RESULT_OF0_DWA2005511_HPP

#include <dgboost/parameter/aux_/use_default_tag.hpp>
#include <dgboost/parameter/config.hpp>
#include <dgboost/utility/result_of.hpp>

#if defined(BOOST_PARAMETER_CAN_USE_MP11)
#include <dgboost/mp11/utility.hpp>
#include <type_traits>
#else
#include <dgboost/mpl/if.hpp>
#include <dgboost/type_traits/is_void.hpp>
#endif

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace parameter { namespace aux {

    // A metafunction returning the result of invoking
    // a nullary function object of the given type.
    template <typename F>
    class result_of0
    {
#if defined(BOOST_NO_RESULT_OF)
        typedef typename F::result_type result_of_F;
#else
        typedef typename ::dgboost::result_of<F()>::type result_of_F;
#endif

     public:
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
        using type = ::dgboost::mp11::mp_if<
            ::std::is_void<result_of_F>
#else
        typedef typename ::dgboost::mpl::if_<
            ::dgboost::is_void<result_of_F>
#endif
          , ::dgboost::parameter::aux::use_default_tag
          , result_of_F
#if defined(BOOST_PARAMETER_CAN_USE_MP11)
        >;
#else
        >::type type;
#endif
    };
}}} // namespace dgboost::parameter::aux

#endif  // include guard

