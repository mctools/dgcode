// Copyright David Abrahams 2002.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef RETURN_INTERNAL_REFERENCE_DWA2002131_HPP
# define RETURN_INTERNAL_REFERENCE_DWA2002131_HPP

# include <dgboost/python/detail/prefix.hpp>

# include <dgboost/python/default_call_policies.hpp>
# include <dgboost/python/reference_existing_object.hpp>
# include <dgboost/python/with_custodian_and_ward.hpp>
# include <dgboost/mpl/if.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python { 

namespace detail
{
  template <std::size_t>
  struct return_internal_reference_owner_arg_must_be_greater_than_zero
# if defined(__GNUC__) || defined(__EDG__)
  {}
# endif
  ;
}

template <std::size_t owner_arg = 1, class BasePolicy_ = default_call_policies>
struct return_internal_reference
    : with_custodian_and_ward_postcall<0, owner_arg, BasePolicy_>
{
 private:
    BOOST_STATIC_CONSTANT(bool, legal = owner_arg > 0);
 public:
    typedef typename mpl::if_c<
        legal
        , reference_existing_object
        , detail::return_internal_reference_owner_arg_must_be_greater_than_zero<owner_arg>
    >::type result_converter;
};

}} // namespace dgboost::python

#endif // RETURN_INTERNAL_REFERENCE_DWA2002131_HPP
