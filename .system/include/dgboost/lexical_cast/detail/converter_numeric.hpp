// Copyright Kevlin Henney, 2000-2005.
// Copyright Alexander Nasonov, 2006-2010.
// Copyright Antony Polukhin, 2011-2019.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// what:  lexical_cast custom keyword cast
// who:   contributed by Kevlin Henney,
//        enhanced with contributions from Terje Slettebo,
//        with additional fixes and suggestions from Gennaro Prota,
//        Beman Dawes, Dave Abrahams, Daryle Walker, Peter Dimov,
//        Alexander Nasonov, Antony Polukhin, Justin Viiret, Michael Hofmann,
//        Cheng Yang, Matthew Bradbury, David W. Birdsall, Pavel Korzh and other Boosters
// when:  November 2000, March 2003, June 2005, June 2006, March 2011 - 2016

#ifndef BOOST_LEXICAL_CAST_DETAIL_CONVERTER_NUMERIC_HPP
#define BOOST_LEXICAL_CAST_DETAIL_CONVERTER_NUMERIC_HPP

#include <dgboost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include <dgboost/limits.hpp>
#include <dgboost/type_traits/type_identity.hpp>
#include <dgboost/type_traits/conditional.hpp>
#include <dgboost/type_traits/make_unsigned.hpp>
#include <dgboost/type_traits/is_signed.hpp>
#include <dgboost/type_traits/is_integral.hpp>
#include <dgboost/type_traits/is_arithmetic.hpp>
#include <dgboost/type_traits/is_base_of.hpp>
#include <dgboost/type_traits/is_float.hpp>

#include <dgboost/numeric/conversion/cast.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace detail {

template <class Source >
struct detect_precision_loss
{
    typedef Source source_type;
    typedef dgboost::numeric::Trunc<Source> Rounder;
    typedef BOOST_DEDUCED_TYPENAME conditional<
        dgboost::is_arithmetic<Source>::value, Source, Source const&
    >::type argument_type ;

    static inline source_type nearbyint(argument_type s, bool& is_ok) BOOST_NOEXCEPT {
        const source_type near_int = Rounder::nearbyint(s);
        if (near_int && is_ok) {
            const source_type orig_div_round = s / near_int;
            const source_type eps = std::numeric_limits<source_type>::epsilon();

            is_ok = !((orig_div_round > 1 ? orig_div_round - 1 : 1 - orig_div_round) > eps);
        }

        return s;
    }

    typedef typename Rounder::round_style round_style;
};

template <typename Base, class Source>
struct fake_precision_loss: public Base
{
    typedef Source source_type ;
    typedef BOOST_DEDUCED_TYPENAME conditional<
        dgboost::is_arithmetic<Source>::value, Source, Source const&
    >::type argument_type ;

    static inline source_type nearbyint(argument_type s, bool& /*is_ok*/) BOOST_NOEXCEPT {
        return s;
    }
};

struct nothrow_overflow_handler
{
    inline bool operator() ( dgboost::numeric::range_check_result r ) const BOOST_NOEXCEPT {
        return (r == dgboost::numeric::cInRange);
    }
};

template <typename Target, typename Source>
inline bool noexcept_numeric_convert(const Source& arg, Target& result) BOOST_NOEXCEPT {
    typedef dgboost::numeric::converter<
            Target,
            Source,
            dgboost::numeric::conversion_traits<Target, Source >,
            nothrow_overflow_handler,
            detect_precision_loss<Source >
    > converter_orig_t;

    typedef BOOST_DEDUCED_TYPENAME dgboost::conditional<
        dgboost::is_base_of< detect_precision_loss<Source >, converter_orig_t >::value,
        converter_orig_t,
        fake_precision_loss<converter_orig_t, Source>
    >::type converter_t;

    bool res = nothrow_overflow_handler()(converter_t::out_of_range(arg));
    result = converter_t::low_level_convert(converter_t::nearbyint(arg, res));
    return res;
}

template <typename Target, typename Source>
struct lexical_cast_dynamic_num_not_ignoring_minus
{
    static inline bool try_convert(const Source &arg, Target& result) BOOST_NOEXCEPT {
        return noexcept_numeric_convert<Target, Source >(arg, result);
    }
};

template <typename Target, typename Source>
struct lexical_cast_dynamic_num_ignoring_minus
{
    static inline bool try_convert(const Source &arg, Target& result) BOOST_NOEXCEPT {
        typedef BOOST_DEDUCED_TYPENAME dgboost::conditional<
                dgboost::is_float<Source>::value,
                dgboost::type_identity<Source>,
                dgboost::make_unsigned<Source>
        >::type usource_lazy_t;
        typedef BOOST_DEDUCED_TYPENAME usource_lazy_t::type usource_t;

        if (arg < 0) {
            const bool res = noexcept_numeric_convert<Target, usource_t>(0u - arg, result);
            result = static_cast<Target>(0u - result);
            return res;
        } else {
            return noexcept_numeric_convert<Target, usource_t>(arg, result);
        }
    }
};

/*
 * lexical_cast_dynamic_num follows the rules:
 * 1) If Source can be converted to Target without precision loss and
 * without overflows, then assign Source to Target and return
 *
 * 2) If Source is less than 0 and Target is an unsigned integer,
 * then negate Source, check the requirements of rule 1) and if
 * successful, assign static_casted Source to Target and return
 *
 * 3) Otherwise throw a bad_lexical_cast exception
 *
 *
 * Rule 2) required because dgboost::lexical_cast has the behavior of
 * stringstream, which uses the rules of scanf for conversions. And
 * in the C99 standard for unsigned input value minus sign is
 * optional, so if a negative number is read, no errors will arise
 * and the result will be the two's complement.
 */
template <typename Target, typename Source>
struct dynamic_num_converter_impl
{
    static inline bool try_convert(const Source &arg, Target& result) BOOST_NOEXCEPT {
        typedef BOOST_DEDUCED_TYPENAME dgboost::conditional<
            dgboost::is_unsigned<Target>::value &&
            (dgboost::is_signed<Source>::value || dgboost::is_float<Source>::value) &&
            !(dgboost::is_same<Source, bool>::value) &&
            !(dgboost::is_same<Target, bool>::value),
            lexical_cast_dynamic_num_ignoring_minus<Target, Source>,
            lexical_cast_dynamic_num_not_ignoring_minus<Target, Source>
        >::type caster_type;

        return caster_type::try_convert(arg, result);
    }
};

}} // namespace dgboost::detail

#endif // BOOST_LEXICAL_CAST_DETAIL_CONVERTER_NUMERIC_HPP

