// Copyright David Abrahams 2002.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef IMPLICIT_DWA2002325_HPP
# define IMPLICIT_DWA2002325_HPP

# include <dgboost/python/detail/prefix.hpp>
# include <dgboost/type.hpp>
# include <dgboost/python/converter/implicit.hpp>
# include <dgboost/python/converter/registry.hpp>
#ifndef BOOST_PYTHON_NO_PY_SIGNATURES
# include <dgboost/python/converter/pytype_function.hpp>
#endif
# include <dgboost/python/type_id.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python { 

template <class Source, class Target>
void implicitly_convertible(dgboost::type<Source>* = 0, dgboost::type<Target>* = 0)
{
    typedef converter::implicit<Source,Target> functions;
    
    converter::registry::push_back(
          &functions::convertible
        , &functions::construct
        , type_id<Target>()
#ifndef BOOST_PYTHON_NO_PY_SIGNATURES
        , &converter::expected_from_python_type_direct<Source>::get_pytype
#endif
        );
}

}} // namespace dgboost::python

#endif // IMPLICIT_DWA2002325_HPP
