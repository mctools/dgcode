
#ifndef BOOST_MPL_REMOVE_IF_HPP_INCLUDED
#define BOOST_MPL_REMOVE_IF_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
// Copyright David Abrahams 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id$
// $Date$
// $Revision$

#include <dgboost/mpl/fold.hpp>
#include <dgboost/mpl/reverse_fold.hpp>
#include <dgboost/mpl/eval_if.hpp>
#include <dgboost/mpl/identity.hpp>
#include <dgboost/mpl/protect.hpp>
#include <dgboost/mpl/lambda.hpp>
#include <dgboost/mpl/apply.hpp>
#include <dgboost/mpl/aux_/inserter_algorithm.hpp>

namespace dgboost {} namespace boost = dgboost; namespace dgboost { namespace mpl {

namespace aux {

template< typename Pred, typename InsertOp > struct remove_if_helper
{
    template< typename Sequence, typename U > struct apply
    {
        typedef typename eval_if<
              typename apply1<Pred,U>::type
            , identity<Sequence>
            , apply2<InsertOp,Sequence,U>
            >::type type;
    };
};

template<
      typename Sequence
    , typename Predicate
    , typename Inserter
    >
struct remove_if_impl
    : fold<
          Sequence
        , typename Inserter::state
        , protect< aux::remove_if_helper<
              typename lambda<Predicate>::type
            , typename Inserter::operation
            > >
        >
{
};

template<
      typename Sequence
    , typename Predicate
    , typename Inserter
    >
struct reverse_remove_if_impl
    : reverse_fold<
          Sequence
        , typename Inserter::state
        , protect< aux::remove_if_helper<
              typename lambda<Predicate>::type
            , typename Inserter::operation
            > >
        >
{
};

} // namespace aux

BOOST_MPL_AUX_INSERTER_ALGORITHM_DEF(3, remove_if)

}}

#endif // BOOST_MPL_REMOVE_IF_HPP_INCLUDED
