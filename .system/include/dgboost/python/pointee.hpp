// Copyright David Abrahams 2002.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef POINTEE_DWA2002323_HPP
# define POINTEE_DWA2002323_HPP

# include <dgboost/python/detail/prefix.hpp>
# include <dgboost/python/detail/type_traits.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python {

namespace detail
{
  template <bool is_ptr = true>
  struct pointee_impl
  {
      template <class T> struct apply : detail::remove_pointer<T> {};
  };

  template <>
  struct pointee_impl<false>
  {
      template <class T> struct apply
      {
          typedef typename T::element_type type;
      };
  };
}

template <class T>
struct pointee
    : detail::pointee_impl<
        detail::is_pointer<T>::value
      >::template apply<T>
{
};

}} // namespace dgboost::python

#endif // POINTEE_DWA2002323_HPP
