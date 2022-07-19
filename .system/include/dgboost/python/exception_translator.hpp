// Copyright David Abrahams 2002.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef EXCEPTION_TRANSLATOR_DWA2002810_HPP
# define EXCEPTION_TRANSLATOR_DWA2002810_HPP

# include <dgboost/python/detail/prefix.hpp>

# include <dgboost/bind.hpp>
# include <dgboost/bind/placeholders.hpp>
# include <dgboost/type.hpp>
# include <dgboost/python/detail/translate_exception.hpp>
# include <dgboost/python/detail/exception_handler.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace python { 

template <class ExceptionType, class Translate>
void register_exception_translator(Translate translate, dgboost::type<ExceptionType>* = 0)
{
    detail::register_exception_handler(
        dgboost::bind<bool>(detail::translate_exception<ExceptionType,Translate>(), _1, _2, translate)
        );
}

}} // namespace dgboost::python

#endif // EXCEPTION_TRANSLATOR_DWA2002810_HPP
