/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5FILEDRIVER_MISC_HPP
#define H5FILEDRIVER_MISC_HPP

#include "../H5FileDriver.hpp"

#include <H5Ppublic.h>

#ifdef H5_HAVE_PARALLEL
#include <H5FDmpi.h>
#endif

namespace HighFive {

namespace {

class DefaultFileDriver : public FileDriver {
  public:
    inline DefaultFileDriver() : FileDriver(H5P_DEFAULT) {}
};
}

// file access property
inline FileDriver::FileDriver() { _hid = H5Pcreate(H5P_FILE_ACCESS); }

inline FileDriver::FileDriver(hid_t fapl) { _hid = fapl; }

template <typename Comm, typename Info>
inline MPIOFileDriver::MPIOFileDriver(Comm comm, Info info) {
    if (H5Pset_fapl_mpio(_hid, comm, info) < 0) {
        HDF5ErrMapper::ToException<FileException>(
            "Unable to setup MPIO Driver configuration");
    }
}
}

#endif // H5FILEDRIVER_MISC_HPP
