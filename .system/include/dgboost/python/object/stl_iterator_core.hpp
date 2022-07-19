// Copyright Eric Niebler 2005.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef STL_ITERATOR_CORE_EAN20051028_HPP
# define STL_ITERATOR_CORE_EAN20051028_HPP

# include <dgboost/python/object_fwd.hpp>
# include <dgboost/python/handle_fwd.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python { namespace objects {

struct BOOST_PYTHON_DECL stl_input_iterator_impl
{
    stl_input_iterator_impl();
    stl_input_iterator_impl(dgboost::python::object const &ob);
    void increment();
    bool equal(stl_input_iterator_impl const &that) const;
    dgboost::python::handle<> const &current() const;
private:
    dgboost::python::object it_;
    dgboost::python::handle<> ob_;
};

}}} // namespace dgboost::python::object

#endif // STL_ITERATOR_CORE_EAN20051028_HPP
