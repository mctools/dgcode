// Copyright (C) 2006 Douglas Gregor <doug.gregor -at- gmail.com>.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Message Passing Interface

//  See www.boost.org/libs/mpi for documentation.

/** @file mpi.hpp
 *
 *  This file is a top-level convenience header that includes all of
 *  the Boost.MPI library headers. Users concerned about compile time
 *  may wish to include only specific headers from the Boost.MPI
 *  library.
 *
 */
#ifndef BOOST_MPI_HPP
#define BOOST_MPI_HPP

#include <dgboost/mpi/allocator.hpp>
#include <dgboost/mpi/collectives.hpp>
#include <dgboost/mpi/communicator.hpp>
#include <dgboost/mpi/datatype.hpp>
#include <dgboost/mpi/environment.hpp>
#include <dgboost/mpi/graph_communicator.hpp>
#include <dgboost/mpi/group.hpp>
#include <dgboost/mpi/intercommunicator.hpp>
#include <dgboost/mpi/nonblocking.hpp>
#include <dgboost/mpi/operations.hpp>
#include <dgboost/mpi/skeleton_and_content.hpp>
#include <dgboost/mpi/timer.hpp>

#endif // BOOST_MPI_HPP
