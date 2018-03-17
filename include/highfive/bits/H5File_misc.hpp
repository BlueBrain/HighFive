/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5FILE_MISC_HPP
#define H5FILE_MISC_HPP

#include "../H5Exception.hpp"
#include "../H5File.hpp"

#include <H5Fpublic.h>

namespace HighFive {

namespace {

// libhdf5 uses a preprocessor trick on their oflags
// we can not declare them constant without a mapper
inline int convert_open_flag(int openFlags) {
    int res_open = 0;
    if (openFlags & File::ReadOnly)
        res_open |= H5F_ACC_RDONLY;
    if (openFlags & File::ReadWrite)
        res_open |= H5F_ACC_RDWR;
    if (openFlags & File::Create)
        res_open |= H5F_ACC_CREAT;
    if (openFlags & File::Truncate)
        res_open |= H5F_ACC_TRUNC;
    if (openFlags & File::Excl)
        res_open |= H5F_ACC_EXCL;
    return res_open;
}
}

inline File::File(const std::string& filename, int openFlags,
                  const FileDriver& driver)
    : _filename(filename) {

    openFlags = convert_open_flag(openFlags);

    if (openFlags & (H5F_ACC_CREAT | H5F_ACC_TRUNC)) {
        if ((_hid = H5Fcreate(_filename.c_str(), openFlags & (H5F_ACC_TRUNC),
                              H5P_DEFAULT, driver.getId())) < 0) {
            HDF5ErrMapper::ToException<FileException>(
                std::string("Unable to create file " + _filename));
        }
    } else {
        if ((_hid = H5Fopen(_filename.c_str(), openFlags, driver.getId())) <
            0) {
            HDF5ErrMapper::ToException<FileException>(
                std::string("Unable to open file " + _filename));
        }
    }
}

inline const std::string& File::getName() const {
    return _filename;
}

inline void File::flush() {
    if (H5Fflush(_hid, H5F_SCOPE_GLOBAL) < 0) {
        HDF5ErrMapper::ToException<FileException>(
            std::string("Unable to flush file " + _filename));
    }
}
}

#endif // H5FILE_MISC_HPP
