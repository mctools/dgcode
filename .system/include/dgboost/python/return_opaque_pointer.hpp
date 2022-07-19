// Copyright Gottfried Ganßauge 2003.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
/*
 * Generic Return value converter generator for opaque C++-pointers
 */
# ifndef RETURN_OPAQUE_POINTER_HPP_
# define RETURN_OPAQUE_POINTER_HPP_

# include <dgboost/python/detail/prefix.hpp>
# include <dgboost/python/opaque_pointer_converter.hpp>
# include <dgboost/python/detail/force_instantiate.hpp>
# include <dgboost/python/to_python_value.hpp>
# include <dgboost/python/detail/value_arg.hpp>
# include <dgboost/mpl/assert.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python {

namespace detail
{
  template <class Pointee>
  static void opaque_pointee(Pointee const volatile*)
  {
      force_instantiate(opaque<Pointee>::instance);
  }
}

struct return_opaque_pointer
{
    template <class R>
    struct apply
    {
        BOOST_MPL_ASSERT_MSG( is_pointer<R>::value, RETURN_OPAQUE_POINTER_EXPECTS_A_POINTER_TYPE, (R));
        
        struct type :  
          dgboost::python::to_python_value<
              typename detail::value_arg<R>::type
          >
        {
            type() { detail::opaque_pointee(R()); }
        };
    };
};

}} // namespace dgboost::python
# endif // RETURN_OPAQUE_POINTER_HPP_
