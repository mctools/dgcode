// Copyright Stefan Seefeld 2005.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <dgboost/python/import.hpp>
#include <dgboost/python/borrowed.hpp>
#include <dgboost/python/extract.hpp>
#include <dgboost/python/handle.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost 
{ 
namespace python 
{

object BOOST_PYTHON_DECL import(str name)
{
  // should be 'char const *' but older python versions don't use 'const' yet.
  char *n = python::extract<char *>(name);
  python::handle<> module(PyImport_ImportModule(n));
  return python::object(module);
}

}  // namespace dgboost::python
}  // namespace dgboost
