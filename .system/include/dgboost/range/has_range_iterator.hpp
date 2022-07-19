// Boost.Range library
//
//  Copyright Neil Groves 2010. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//
// Acknowledgments:
// Ticket #8341: Arno Schoedl - improved handling of has_range_iterator upon
// use-cases where T was const.
#ifndef BOOST_RANGE_HAS_ITERATOR_HPP_INCLUDED
#define BOOST_RANGE_HAS_ITERATOR_HPP_INCLUDED

#include <dgboost/mpl/bool.hpp>
#include <dgboost/mpl/eval_if.hpp>
#include <dgboost/mpl/has_xxx.hpp>
#include <dgboost/range/iterator.hpp>
#include <dgboost/type_traits/remove_reference.hpp>
#include <dgboost/utility/enable_if.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost
{
    namespace range_detail
    {
        BOOST_MPL_HAS_XXX_TRAIT_DEF(type)

        template<class T, class Enabler = void>
        struct has_range_iterator_impl
            : dgboost::mpl::false_
        {
        };

        template<class T>
        struct has_range_iterator_impl<
            T,
            BOOST_DEDUCED_TYPENAME ::dgboost::enable_if<
                BOOST_DEDUCED_TYPENAME mpl::eval_if<is_const<T>,
                    has_type<dgboost::range_const_iterator<
                                BOOST_DEDUCED_TYPENAME remove_const<T>::type> >,
                    has_type<dgboost::range_mutable_iterator<T> >
                >::type
            >::type
        >
            : dgboost::mpl::true_
        {
        };

        template<class T, class Enabler = void>
        struct has_range_const_iterator_impl
            : dgboost::mpl::false_
        {
        };

        template<class T>
        struct has_range_const_iterator_impl<
            T,
            BOOST_DEDUCED_TYPENAME ::dgboost::enable_if<
                has_type<dgboost::range_const_iterator<T> >
            >::type
        >
            : dgboost::mpl::true_
        {
        };

    } // namespace range_detail

    template<class T>
    struct has_range_iterator
        : range_detail::has_range_iterator_impl<
            BOOST_DEDUCED_TYPENAME remove_reference<T>::type>
    {};

    template<class T>
    struct has_range_const_iterator
        : range_detail::has_range_const_iterator_impl<
            BOOST_DEDUCED_TYPENAME remove_reference<T>::type>
    {};
} // namespace dgboost

#endif // include guard

