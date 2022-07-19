// Copyright David Abrahams 2003.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef INHERITANCE_QUERY_DWA2003520_HPP
# define INHERITANCE_QUERY_DWA2003520_HPP

# include <dgboost/python/type_id.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python { namespace objects {

BOOST_PYTHON_DECL void* find_static_type(void* p, type_info src, type_info dst);
BOOST_PYTHON_DECL void* find_dynamic_type(void* p, type_info src, type_info dst);

}}} // namespace dgboost::python::object

#endif // INHERITANCE_QUERY_DWA2003520_HPP
