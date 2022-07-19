#if !defined(BOOST_PP_IS_ITERATING)

// Copyright David Abrahams 2001.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

# ifndef POINTER_HOLDER_DWA20011215_HPP
#  define POINTER_HOLDER_DWA20011215_HPP 

# include <dgboost/get_pointer.hpp>
#  include <dgboost/type.hpp>

#  include <dgboost/python/instance_holder.hpp>
#  include <dgboost/python/object/inheritance_query.hpp>
#  include <dgboost/python/object/forward.hpp>

#  include <dgboost/python/pointee.hpp>
#  include <dgboost/python/type_id.hpp>

#  include <dgboost/python/detail/wrapper_base.hpp>
#  include <dgboost/python/detail/force_instantiate.hpp>
#  include <dgboost/python/detail/preprocessor.hpp>
# include <dgboost/python/detail/type_traits.hpp>


#  include <dgboost/mpl/if.hpp>
#  include <dgboost/mpl/apply.hpp>

#  include <dgboost/preprocessor/comma_if.hpp>
#  include <dgboost/preprocessor/iterate.hpp>
#  include <dgboost/preprocessor/repeat.hpp>
#  include <dgboost/preprocessor/debug/line.hpp>
#  include <dgboost/preprocessor/enum_params.hpp>
#  include <dgboost/preprocessor/repetition/enum_binary_params.hpp>

#  include <dgboost/detail/workaround.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python {

template <class T> class wrapper;

}}


namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python { namespace objects {

#define BOOST_PYTHON_UNFORWARD_LOCAL(z, n, _) BOOST_PP_COMMA_IF(n) objects::do_unforward(a##n,0)

template <class Pointer, class Value>
struct pointer_holder : instance_holder
{
    typedef Value value_type;
    
    pointer_holder(Pointer);

    // Forward construction to the held object

#  define BOOST_PP_ITERATION_PARAMS_1 (4, (0, BOOST_PYTHON_MAX_ARITY, <dgboost/python/object/pointer_holder.hpp>, 1))
#  include BOOST_PP_ITERATE()

 private: // types
    
 private: // required holder implementation
    void* holds(type_info, bool null_ptr_only);
    
    template <class T>
    inline void* holds_wrapped(type_info dst_t, wrapper<T>*,T* p)
    {
        return python::type_id<T>() == dst_t ? p : 0;
    }
    
    inline void* holds_wrapped(type_info, ...)
    {
        return 0;
    }

 private: // data members
    Pointer m_p;
};

template <class Pointer, class Value>
struct pointer_holder_back_reference : instance_holder
{
 private:
    typedef typename python::pointee<Pointer>::type held_type;
 public:
    typedef Value value_type;

    // Not sure about this one -- can it work? The source object
    // undoubtedly does not carry the correct back reference pointer.
    pointer_holder_back_reference(Pointer);

    // Forward construction to the held object
#  define BOOST_PP_ITERATION_PARAMS_1 (4, (0, BOOST_PYTHON_MAX_ARITY, <dgboost/python/object/pointer_holder.hpp>, 2))
#  include BOOST_PP_ITERATE()

 private: // required holder implementation
    void* holds(type_info, bool null_ptr_only);

 private: // data members
    Pointer m_p;
};

#  undef BOOST_PYTHON_UNFORWARD_LOCAL

template <class Pointer, class Value>
inline pointer_holder<Pointer,Value>::pointer_holder(Pointer p)
#if defined(BOOST_NO_CXX11_SMART_PTR)
    : m_p(p)
#else
    : m_p(std::move(p))
#endif
{
}

template <class Pointer, class Value>
inline pointer_holder_back_reference<Pointer,Value>::pointer_holder_back_reference(Pointer p)
#if defined(BOOST_NO_CXX11_SMART_PTR)
    : m_p(p)
#else
    : m_p(std::move(p))
#endif
{
}

template <class Pointer, class Value>
void* pointer_holder<Pointer, Value>::holds(type_info dst_t, bool null_ptr_only)
{
    typedef typename dgboost::python::detail::remove_const< Value >::type non_const_value;

    if (dst_t == python::type_id<Pointer>()
        && !(null_ptr_only && get_pointer(this->m_p))
    )
        return &this->m_p;

    Value* p0
#  if BOOST_WORKAROUND(__SUNPRO_CC, BOOST_TESTED_AT(0x590))
        = static_cast<Value*>( get_pointer(this->m_p) )
#  else 
        = get_pointer(this->m_p)
#  endif
        ;
    non_const_value* p = const_cast<non_const_value*>( p0 );

    if (p == 0)
        return 0;
    
    if (void* wrapped = holds_wrapped(dst_t, p, p))
        return wrapped;
    
    type_info src_t = python::type_id<non_const_value>();
    return src_t == dst_t ? p : find_dynamic_type(p, src_t, dst_t);
}

template <class Pointer, class Value>
void* pointer_holder_back_reference<Pointer, Value>::holds(type_info dst_t, bool null_ptr_only)
{
    if (dst_t == python::type_id<Pointer>()
        && !(null_ptr_only && get_pointer(this->m_p))
    )
        return &this->m_p;

    if (!get_pointer(this->m_p))
        return 0;
    
    Value* p = get_pointer(m_p);
    
    if (dst_t == python::type_id<held_type>())
        return p;

    type_info src_t = python::type_id<Value>();
    return src_t == dst_t ? p : find_dynamic_type(p, src_t, dst_t);
}

}}} // namespace dgboost::python::objects

# endif // POINTER_HOLDER_DWA20011215_HPP

/* --------------- pointer_holder --------------- */
// For gcc 4.4 compatability, we must include the
// BOOST_PP_ITERATION_DEPTH test inside an #else clause.
#else // BOOST_PP_IS_ITERATING
#if BOOST_PP_ITERATION_DEPTH() == 1 && BOOST_PP_ITERATION_FLAGS() == 1
# if !(BOOST_WORKAROUND(__MWERKS__, > 0x3100)                      \
        && BOOST_WORKAROUND(__MWERKS__, BOOST_TESTED_AT(0x3201)))
#  line BOOST_PP_LINE(__LINE__, pointer_holder.hpp)
# endif

# define N BOOST_PP_ITERATION()

# if (N != 0)
    template< BOOST_PP_ENUM_PARAMS_Z(1, N, class A) >
# endif
    pointer_holder(PyObject* self BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_BINARY_PARAMS_Z(1, N, A, a))
        : m_p(new Value(
                BOOST_PP_REPEAT_1ST(N, BOOST_PYTHON_UNFORWARD_LOCAL, nil)
            ))
    {
        python::detail::initialize_wrapper(self, get_pointer(this->m_p));
    }

# undef N

/* --------------- pointer_holder_back_reference --------------- */
#elif BOOST_PP_ITERATION_DEPTH() == 1 && BOOST_PP_ITERATION_FLAGS() == 2
# if !(BOOST_WORKAROUND(__MWERKS__, > 0x3100)                      \
        && BOOST_WORKAROUND(__MWERKS__, BOOST_TESTED_AT(0x3201)))
#  line BOOST_PP_LINE(__LINE__, pointer_holder.hpp(pointer_holder_back_reference))
# endif 

# define N BOOST_PP_ITERATION()

# if (N != 0)
    template < BOOST_PP_ENUM_PARAMS_Z(1, N, class A) >
# endif
    pointer_holder_back_reference(
        PyObject* p BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_BINARY_PARAMS_Z(1, N, A, a))
        : m_p(new held_type(
                    p BOOST_PP_COMMA_IF(N) BOOST_PP_REPEAT_1ST(N, BOOST_PYTHON_UNFORWARD_LOCAL, nil)
            ))
    {}

# undef N

#endif // BOOST_PP_ITERATION_DEPTH()
#endif
