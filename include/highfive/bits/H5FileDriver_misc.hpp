/*
 *  Copyright (c), 2017-2018, Adrien Devresse <adrien.devresse@epfl.ch>
 *                            Juan Hernando <juan.hernando@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

namespace HighFive {

#ifdef H5_HAVE_PARALLEL
inline MPIOFileDriver::MPIOFileDriver(MPI_Comm comm, MPI_Info info) {
    add(MPIOFileAccess(comm, info));
}
#endif

}  // namespace HighFive
