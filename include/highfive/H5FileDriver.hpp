/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5FILEDRIVER_HPP
#define H5FILEDRIVER_HPP

#include "H5PropertyList.hpp"

namespace HighFive {

///
/// \brief file driver base concept
///
class FileDriver : public FileAccessProps {};

///
/// \brief MPIIO Driver for Parallel HDF5
///
class MPIOFileDriver : public FileAccessProps {
  public:
    template <typename Comm, typename Info>
    inline MPIOFileDriver(Comm mpi_comm, Info mpi_info);

  private:
};

///
/// \brief Configure the version bounds for the file
///
/// Used to define the compatibility of objects created within HDF5 files,
/// and affects the format of groups stored in the file.
///
/// See also the documentation of \c H5P_SET_LIBVER_BOUNDS in HDF5.
///
/// Possible values for \c low and \c high are:
/// * \c H5F_LIBVER_EARLIEST
/// * \c H5F_LIBVER_V18
/// * \c H5F_LIBVER_V110
/// * \c H5F_LIBVER_NBOUNDS
/// * \c H5F_LIBVER_LATEST currently defined as \c H5F_LIBVER_V110 within
///   HDF5
///
class FileVersionBounds {
  public:
    FileVersionBounds(H5F_libver_t low, H5F_libver_t high)
        : _low(low)
        , _high(high)
    {}
  private:
    friend FileAccessProps;
    void apply(const hid_t list) const;
    const H5F_libver_t _low;
    const H5F_libver_t _high;
};

///
/// \brief Configure the metadata block size to use writing to files
///
/// \param size Metadata block size in bytes
///
class MetadataBlockSize {
  public:
    MetadataBlockSize(hsize_t size)
        : _size(size)
    {}
  private:
    friend FileAccessProps;
    void apply(const hid_t list) const;
    const hsize_t _size;
};

}  // namespace HighFive

#include "bits/H5FileDriver_misc.hpp"

#endif // H5FILEDRIVER_HPP
