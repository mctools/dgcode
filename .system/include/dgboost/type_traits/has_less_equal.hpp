//  (C) Copyright 2009-2011 Frederic Bron.
//
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef BOOST_TT_HAS_LESS_EQUAL_HPP_INCLUDED
#define BOOST_TT_HAS_LESS_EQUAL_HPP_INCLUDED

#define BOOST_TT_TRAIT_NAME has_less_equal
#define BOOST_TT_TRAIT_OP <=
#define BOOST_TT_FORBIDDEN_IF\
   (\
      /* Lhs==pointer and Rhs==fundamental */\
      (\
         ::dgboost::is_pointer< Lhs_noref >::value && \
         ::dgboost::is_fundamental< Rhs_nocv >::value\
      ) || \
      /* Rhs==pointer and Lhs==fundamental */\
      (\
         ::dgboost::is_pointer< Rhs_noref >::value && \
         ::dgboost::is_fundamental< Lhs_nocv >::value\
      ) || \
      /* Lhs==pointer and Rhs==pointer and Lhs!=base(Rhs) and Rhs!=base(Lhs) and Lhs!=void* and Rhs!=void* */\
      (\
         ::dgboost::is_pointer< Lhs_noref >::value && \
         ::dgboost::is_pointer< Rhs_noref >::value && \
         (! \
            ( \
               ::dgboost::is_base_of< Lhs_noptr, Rhs_noptr >::value || \
               ::dgboost::is_base_of< Rhs_noptr, Lhs_noptr >::value || \
               ::dgboost::is_same< Lhs_noptr, Rhs_noptr >::value || \
               ::dgboost::is_void< Lhs_noptr >::value || \
               ::dgboost::is_void< Rhs_noptr >::value\
            )\
         )\
      ) || \
      (\
         ::dgboost::type_traits_detail::is_likely_stateless_lambda<Lhs_noref>::value\
      )\
      )


#include <dgboost/type_traits/detail/has_binary_operator.hpp>

#undef BOOST_TT_TRAIT_NAME
#undef BOOST_TT_TRAIT_OP
#undef BOOST_TT_FORBIDDEN_IF

#endif
