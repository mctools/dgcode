// Boost.Range library
//
// Copyright Neil Groves 2014.
//
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt).
//
// For more information, see http://www.boost.org/libs/range/
//
#ifndef BOOST_RANGE_DETAIL_HAS_MEMBER_SIZE_HPP
#define BOOST_RANGE_DETAIL_HAS_MEMBER_SIZE_HPP

#include <dgboost/type_traits/is_class.hpp>
#include <dgboost/type_traits/is_member_function_pointer.hpp>
#include <dgboost/mpl/and.hpp>
#include <dgboost/mpl/bool.hpp>
#include <dgboost/cstdint.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost
{
    namespace range_detail
    {

template<class T>
class has_member_size_impl
{
private:
    template<class U, U>
    class check
    {
    };

    template<class C>
    static dgboost::uint8_t f(check<std::size_t(C::*)(void) const, &C::size>*);

    template<class C>
    static dgboost::uint16_t f(...);

public:
    static const bool value =
        (sizeof(f<T>(0)) == sizeof(dgboost::uint8_t));

    typedef typename mpl::if_c<
        (sizeof(f<T>(0)) == sizeof(dgboost::uint8_t)),
        mpl::true_,
        mpl::false_
    >::type type;
};

template<class T>
struct has_member_size
{
    typedef typename mpl::and_<
        typename is_class<T>::type,
        typename has_member_size_impl<const T>::type
    >::type type;

    static const bool value =
        is_class<T>::value && has_member_size_impl<const T>::value;
};

    } // namespace range_detail
}// namespace dgboost
 
#endif // include guard
