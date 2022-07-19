// Copyright (C) 2005 Arkadiy Vertleyb
// Copyright (C) 2005 Peder Holt
//
// Copyright (C) 2006 Tobias Schwinger
//
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TYPEOF_VECTOR_HPP_INCLUDED

#include <dgboost/typeof/constant.hpp>
#include <dgboost/preprocessor/iteration/self.hpp>

#ifndef BOOST_TYPEOF_LIMIT_SIZE
#   define BOOST_TYPEOF_LIMIT_SIZE 50
#endif

//
// To recreate the preprocessed versions of this file preprocess and run
//
//   $(BOOST_ROOT)/libs/typeof/tools/preprocess.pl
//

#if defined(BOOST_TYPEOF_PP_INCLUDE_EXTERNAL)

#   undef BOOST_TYPEOF_PP_INCLUDE_EXTERNAL

#elif !defined(BOOST_TYPEOF_PREPROCESSING_MODE) && !BOOST_PP_IS_SELFISH

#   define BOOST_PP_INDIRECT_SELF <dgboost/typeof/vector.hpp>
#   if   BOOST_TYPEOF_LIMIT_SIZE < 50
#     include BOOST_PP_INCLUDE_SELF()
#   elif BOOST_TYPEOF_LIMIT_SIZE < 100
#     include <dgboost/typeof/vector50.hpp>
#     define  BOOST_TYPEOF_PP_START_SIZE 51
#     include BOOST_PP_INCLUDE_SELF()
#   elif BOOST_TYPEOF_LIMIT_SIZE < 150
#     include <dgboost/typeof/vector100.hpp>
#     define  BOOST_TYPEOF_PP_START_SIZE 101
#     include BOOST_PP_INCLUDE_SELF()
#   elif BOOST_TYPEOF_LIMIT_SIZE < 200
#     include <dgboost/typeof/vector150.hpp>
#     define  BOOST_TYPEOF_PP_START_SIZE 151
#     include BOOST_PP_INCLUDE_SELF()
#   elif BOOST_TYPEOF_LIMIT_SIZE <= 250
#     include <dgboost/typeof/vector200.hpp>
#     define  BOOST_TYPEOF_PP_START_SIZE 201
#     include BOOST_PP_INCLUDE_SELF()
#   else
#     error "BOOST_TYPEOF_LIMIT_SIZE too high"
#   endif

#else// defined(BOOST_TYPEOF_PREPROCESSING_MODE) || BOOST_PP_IS_SELFISH

#   ifndef BOOST_TYPEOF_PP_NEXT_SIZE
#     define BOOST_TYPEOF_PP_NEXT_SIZE BOOST_TYPEOF_LIMIT_SIZE
#   endif
#   ifndef BOOST_TYPEOF_PP_START_SIZE
#     define BOOST_TYPEOF_PP_START_SIZE 0
#   endif

#   if BOOST_TYPEOF_PP_START_SIZE <= BOOST_TYPEOF_LIMIT_SIZE

#     include <dgboost/preprocessor/enum_params.hpp>
#     include <dgboost/preprocessor/repeat.hpp>
#     include <dgboost/preprocessor/repeat_from_to.hpp>
#     include <dgboost/preprocessor/cat.hpp>
#     include <dgboost/preprocessor/inc.hpp>
#     include <dgboost/preprocessor/dec.hpp>
#     include <dgboost/preprocessor/comma_if.hpp>
#     include <dgboost/preprocessor/iteration/local.hpp>
#     include <dgboost/preprocessor/control/expr_iif.hpp>
#     include <dgboost/preprocessor/logical/not.hpp>

// iterator

#     define BOOST_TYPEOF_spec_iter(n)\
        template<class V>\
        struct v_iter<V, constant<int,n> >\
        {\
            typedef typename V::item ## n type;\
            typedef v_iter<V, constant<int,n + 1> > next;\
        };

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace type_of {

    template<class V, class Increase_BOOST_TYPEOF_LIMIT_SIZE> struct v_iter; // not defined
#     define  BOOST_PP_LOCAL_MACRO  BOOST_TYPEOF_spec_iter
#     define  BOOST_PP_LOCAL_LIMITS \
        (BOOST_PP_DEC(BOOST_TYPEOF_PP_START_SIZE), \
         BOOST_PP_DEC(BOOST_TYPEOF_LIMIT_SIZE))
#     include BOOST_PP_LOCAL_ITERATE()

}}

#     undef BOOST_TYPEOF_spec_iter

// vector

#     define BOOST_TYPEOF_typedef_item(z, n, _)\
        typedef P ## n item ## n;

#     define BOOST_TYPEOF_typedef_fake_item(z, n, _)\
        typedef constant<int,1> item ## n;

#     define BOOST_TYPEOF_define_vector(n)\
        template<BOOST_PP_ENUM_PARAMS(n, class P) BOOST_PP_EXPR_IIF(BOOST_PP_NOT(n), class T = void)>\
        struct vector ## n\
        {\
            typedef v_iter<vector ## n<BOOST_PP_ENUM_PARAMS(n,P)>, dgboost::type_of::constant<int,0> > begin;\
            BOOST_PP_REPEAT(n, BOOST_TYPEOF_typedef_item, ~)\
            BOOST_PP_REPEAT_FROM_TO(n, BOOST_TYPEOF_PP_NEXT_SIZE, BOOST_TYPEOF_typedef_fake_item, ~)\
        };

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace type_of {

#     define  BOOST_PP_LOCAL_MACRO  BOOST_TYPEOF_define_vector
#     define  BOOST_PP_LOCAL_LIMITS \
        (BOOST_TYPEOF_PP_START_SIZE,BOOST_TYPEOF_LIMIT_SIZE)
#     include BOOST_PP_LOCAL_ITERATE()

}}

#     undef BOOST_TYPEOF_typedef_item
#     undef BOOST_TYPEOF_typedef_fake_item
#     undef BOOST_TYPEOF_define_vector

// push_back

#     define BOOST_TYPEOF_spec_push_back(n)\
        template<BOOST_PP_ENUM_PARAMS(n, class P) BOOST_PP_COMMA_IF(n) class T>\
        struct push_back<BOOST_PP_CAT(dgboost::type_of::vector, n)<BOOST_PP_ENUM_PARAMS(n, P)>, T>\
        {\
            typedef BOOST_PP_CAT(dgboost::type_of::vector, BOOST_PP_INC(n))<\
                BOOST_PP_ENUM_PARAMS(n, P) BOOST_PP_COMMA_IF(n) T\
            > type;\
        };

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace type_of {

#   if   BOOST_TYPEOF_LIMIT_SIZE < 50
    template<class V, class T> struct push_back {
        typedef V type;
    };
#   endif
    //default behaviour is to let push_back ignore T, and return the input vector.
    //This is to let BOOST_TYPEOF_NESTED_TYPEDEF work properly with the default vector.
#     define  BOOST_PP_LOCAL_MACRO  BOOST_TYPEOF_spec_push_back
#     define  BOOST_PP_LOCAL_LIMITS \
        (BOOST_PP_DEC(BOOST_TYPEOF_PP_START_SIZE), \
         BOOST_PP_DEC(BOOST_TYPEOF_LIMIT_SIZE))
#     include BOOST_PP_LOCAL_ITERATE()

}}

#     undef BOOST_TYPEOF_spec_push_back

#   endif//BOOST_TYPEOF_PP_START_SIZE<=BOOST_TYPEOF_LIMIT_SIZE
#   undef  BOOST_TYPEOF_PP_START_SIZE
#   undef  BOOST_TYPEOF_PP_NEXT_SIZE

#endif//BOOST_TYPEOF_PREPROCESSING_MODE || BOOST_PP_IS_SELFISH

#define BOOST_TYPEOF_VECTOR_HPP_INCLUDED
#endif//BOOST_TYPEOF_VECTOR_HPP_INCLUDED

