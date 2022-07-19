// Copyright David Abrahams 2002.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef TO_PYTHON_INDIRECT_DWA200221_HPP
# define TO_PYTHON_INDIRECT_DWA200221_HPP

# include <dgboost/python/detail/prefix.hpp>

# include <dgboost/python/object/pointer_holder.hpp>
# include <dgboost/python/object/make_ptr_instance.hpp>

# include <dgboost/python/detail/none.hpp>

#ifndef BOOST_PYTHON_NO_PY_SIGNATURES
# include <dgboost/python/converter/pytype_function.hpp>
#endif

# include <dgboost/python/refcount.hpp>

# include <dgboost/python/detail/type_traits.hpp>

# if defined(__ICL) && __ICL < 600 
#  include <dgboost/shared_ptr.hpp>
# else 
#  include <memory>
# endif

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python {

template <class T, class MakeHolder>
struct to_python_indirect
{
    template <class U>
    inline PyObject*
    operator()(U const& ref) const
    {
        return this->execute(const_cast<U&>(ref), detail::is_pointer<U>());
    }
#ifndef BOOST_PYTHON_NO_PY_SIGNATURES
    inline PyTypeObject const*
    get_pytype()const
    {
        return converter::registered_pytype<T>::get_pytype();
    }
#endif
 private:
    template <class U>
    inline PyObject* execute(U* ptr, detail::true_) const
    {
        // No special NULL treatment for references
        if (ptr == 0)
            return python::detail::none();
        else
            return this->execute(*ptr, detail::false_());
    }
    
    template <class U>
    inline PyObject* execute(U const& x, detail::false_) const
    {
        U* const p = &const_cast<U&>(x);
        if (detail::is_polymorphic<U>::value)
        {
            if (PyObject* o = detail::wrapper_base_::owner(p))
                return incref(o);
        }
        return MakeHolder::execute(p);
    }
};

//
// implementations
//
namespace detail
{
  struct make_owning_holder
  {
      template <class T>
      static PyObject* execute(T* p)
      {
          // can't use auto_ptr with Intel 5 and VC6 Dinkum library
          // for some reason. We get link errors against the auto_ptr
          // copy constructor.
# if defined(__ICL) && __ICL < 600 
          typedef dgboost::shared_ptr<T> smart_pointer;
# elif defined(BOOST_NO_CXX11_SMART_PTR)
          typedef std::auto_ptr<T> smart_pointer;
# else
          typedef std::unique_ptr<T> smart_pointer;
# endif
          typedef objects::pointer_holder<smart_pointer, T> holder_t;

          smart_pointer ptr(const_cast<T*>(p));
          return objects::make_ptr_instance<T, holder_t>::execute(ptr);
      }
  };

  struct make_reference_holder
  {
      template <class T>
      static PyObject* execute(T* p)
      {
          typedef objects::pointer_holder<T*, T> holder_t;
          T* q = const_cast<T*>(p);
          return objects::make_ptr_instance<T, holder_t>::execute(q);
      }
  };
}

}} // namespace dgboost::python

#endif // TO_PYTHON_INDIRECT_DWA200221_HPP
