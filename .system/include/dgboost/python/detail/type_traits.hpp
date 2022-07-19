// Copyright Shreyans Doshi 2017.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PYTHON_DETAIL_TYPE_TRAITS_HPP
# define BOOST_PYTHON_DETAIL_TYPE_TRAITS_HPP


#include <dgboost/config.hpp>
#ifdef BOOST_NO_CXX11_HDR_TYPE_TRAITS
# include <dgboost/type_traits/transform_traits.hpp>
# include <dgboost/type_traits/same_traits.hpp>
# include <dgboost/type_traits/cv_traits.hpp>
# include <dgboost/type_traits/is_polymorphic.hpp>
# include <dgboost/type_traits/composite_traits.hpp>
# include <dgboost/type_traits/conversion_traits.hpp>
# include <dgboost/type_traits/add_pointer.hpp>
# include <dgboost/type_traits/remove_pointer.hpp>
# include <dgboost/type_traits/is_void.hpp>
# include <dgboost/type_traits/object_traits.hpp>
# include <dgboost/type_traits/add_lvalue_reference.hpp>
# include <dgboost/type_traits/function_traits.hpp>
# include <dgboost/type_traits/is_scalar.hpp>
# include <dgboost/type_traits/alignment_traits.hpp>
# include <dgboost/mpl/bool.hpp>
#else
# include <type_traits>
#endif

# include <dgboost/type_traits/is_base_and_derived.hpp>
# include <dgboost/type_traits/alignment_traits.hpp>
# include <dgboost/type_traits/has_trivial_copy.hpp>


namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python { namespace detail {

#ifdef BOOST_NO_CXX11_HDR_TYPE_TRAITS
    using dgboost::alignment_of;
    using dgboost::add_const;
    using dgboost::add_cv;
    using dgboost::add_lvalue_reference;
    using dgboost::add_pointer;

    using dgboost::is_array;
    using dgboost::is_class;
    using dgboost::is_const;
    using dgboost::is_convertible;
    using dgboost::is_enum;
    using dgboost::is_function;
    using dgboost::is_integral;
    using dgboost::is_lvalue_reference;
    using dgboost::is_member_function_pointer;
    using dgboost::is_member_pointer;
    using dgboost::is_pointer;
    using dgboost::is_polymorphic;
    using dgboost::is_reference;
    using dgboost::is_same;
    using dgboost::is_scalar;
    using dgboost::is_union;
    using dgboost::is_void;
    using dgboost::is_volatile;

    using dgboost::remove_reference;
    using dgboost::remove_pointer;
    using dgboost::remove_cv;
    using dgboost::remove_const;

    using dgboost::mpl::true_;
    using dgboost::mpl::false_;
#else
    using std::alignment_of;
    using std::add_const;
    using std::add_cv;
    using std::add_lvalue_reference;
    using std::add_pointer;

    using std::is_array;
    using std::is_class;
    using std::is_const;
    using std::is_convertible;
    using std::is_enum;
    using std::is_function;
    using std::is_integral;
    using std::is_lvalue_reference;
    using std::is_member_function_pointer;
    using std::is_member_pointer;
    using std::is_pointer;
    using std::is_polymorphic;
    using std::is_reference;
    using std::is_same;
    using std::is_scalar;
    using std::is_union;
    using std::is_void;
    using std::is_volatile;

    using std::remove_reference;
    using std::remove_pointer;
    using std::remove_cv;
    using std::remove_const;

    typedef std::integral_constant<bool, true> true_;
    typedef std::integral_constant<bool, false> false_;
#endif
    using dgboost::is_base_and_derived;
    using dgboost::type_with_alignment;
    using dgboost::has_trivial_copy;
}}} // namespace dgboost::python::detail


#endif //BOOST_DETAIL_TYPE_TRAITS_HPP
