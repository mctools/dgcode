// Copyright Jim Bosch 2010-2012.
// Copyright Stefan Seefeld 2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef boost_python_numpy_hpp_
#define boost_python_numpy_hpp_

#include <dgboost/python/numpy/dtype.hpp>
#include <dgboost/python/numpy/ndarray.hpp>
#include <dgboost/python/numpy/scalars.hpp>
#include <dgboost/python/numpy/matrix.hpp>
#include <dgboost/python/numpy/ufunc.hpp>
#include <dgboost/python/numpy/invoke_matching.hpp>
#include <dgboost/python/numpy/config.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python { namespace numpy {

/**
 *  @brief Initialize the Numpy C-API
 *
 *  This must be called before using anything in boost.numpy;
 *  It should probably be the first line inside BOOST_PYTHON_MODULE.
 *
 *  @internal This just calls the Numpy C-API functions "import_array()"
 *            and "import_ufunc()", and then calls
 *            dtype::register_scalar_converters().
 */
BOOST_NUMPY_DECL void initialize(bool register_scalar_converters=true);

}}} // namespace dgboost::python::numpy

#endif
