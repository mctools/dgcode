// Copyright Nikolay Mladenov 2007.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef FUNCTION_SIGNATURE_20070531_HPP
# define FUNCTION_SIGNATURE_20070531_HPP

#include <dgboost/python/object/function.hpp>
#include <dgboost/python/converter/registrations.hpp>
#include <dgboost/python/str.hpp>
#include <dgboost/python/tuple.hpp>

#include <dgboost/python/detail/signature.hpp>


#include <vector>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python { namespace objects {

class function_doc_signature_generator{
    static const  char * py_type_str(const python::detail::signature_element &s);
    static bool arity_cmp( function const *f1, function const *f2 );
    static bool are_seq_overloads( function const *f1, function const *f2 , bool check_docs);
    static std::vector<function const*> flatten(function const *f);
    static std::vector<function const*> split_seq_overloads( const std::vector<function const *> &funcs, bool split_on_doc_change);
    static str raw_function_pretty_signature(function const *f, size_t n_overloads,  bool cpp_types = false);
    static str parameter_string(py_function const &f, size_t n, object arg_names, bool cpp_types);
    static str pretty_signature(function const *f, size_t n_overloads,  bool cpp_types = false);

public:
    static list function_doc_signatures( function const * f);
};

}}}//end of namespace dgboost::python::objects

#endif //FUNCTION_SIGNATURE_20070531_HPP
