/*
 *  Copyright (c), 2019, Tom de Geus <tom@geus.me>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5EASY_HPP
#define H5EASY_HPP

#include <iostream>
#include <string>
#include <vector>

#include "H5DataSet.hpp"
#include "H5DataSpace.hpp"
#include "H5DataType.hpp"
#include "H5File.hpp"

// optionally enable plug-in xtensor and load the library
#ifdef XTENSOR_VERSION_MAJOR
#define HIGHFIVE_XTENSOR
#endif

#ifdef HIGHFIVE_XTENSOR
#include <xtensor/xarray.hpp>
#include <xtensor/xtensor.hpp>
#endif

// optionally enable plug-in Eigen and load the library
#ifdef EIGEN_WORLD_VERSION
#define HIGHFIVE_EIGEN
#endif

#ifdef HIGHFIVE_EIGEN
#include <Eigen/Eigen>
#endif

namespace H5Easy {

using HighFive::AtomicType;
using HighFive::Chunking;
using HighFive::DataSet;
using HighFive::DataSetCreateProps;
using HighFive::DataSpace;
using HighFive::Exception;
using HighFive::File;

///
/// Write mode for DataSets
///
enum class DumpMode
{
  Create,
  Overwrite
};

///
/// Get the size of an existing DataSet in an open HDF5 file.
///
/// @param file opened File
/// @param path path of the DataSet
///
/// @return size the size of the DataSet
size_t getSize(const File& file, const std::string& path);

///
/// Get the shape of an existing DataSet in an open HDF5 file.
///
/// @param file opened File
/// @param path path of the DataSet
///
/// @return shape the shape of the DataSet
inline std::vector<size_t> getShape(const File& file, const std::string& path);

///
/// Write "Eigen::Matrix<T,Rows,Cols,Options>" to a new DataSet in an open HDF5
/// file.
///
/// @param file opened File (has to be writeable)
/// @param path path of the DataSet
/// @param data the data to write
/// @param mode write mode (DumpMode::Create | DumpMode::Overwrite)
///
/// @return dataset the newly created DataSet (e.g. to add an
/// attribute)
///
#ifdef HIGHFIVE_EIGEN
template <class T, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
inline DataSet dump(File& file,
                    const std::string& path,
                    const Eigen::Matrix<T, Rows, Cols, Options, MaxRows, MaxCols>& data,
                    DumpMode mode = DumpMode::Create);
#endif

///
/// Write "xt::xarray<T>" to a new DataSet in an open HDF5 file.
///
/// @param file opened File (has to be writeable)
/// @param path path of the DataSet
/// @param data the data to write
/// @param mode write mode (DumpMode::Create | DumpMode::Overwrite)
///
/// @return dataset the newly created DataSet (e.g. to add an
/// attribute)
///
#ifdef HIGHFIVE_XTENSOR
template <class T>
inline DataSet dump(File& file,
                    const std::string& path,
                    const xt::xarray<T>& data,
                    DumpMode mode = DumpMode::Create);
#endif

///
/// Write "xt::xtensor<T,rank>" to a new DataSet in an open HDF5 file.
///
/// @param file opened File (has to be writeable)
/// @param path path of the DataSet
/// @param data the data to write
/// @param mode write mode (DumpMode::Create | DumpMode::Overwrite)
///
/// @return dataset the newly created DataSet (e.g. to add an
/// attribute)
///
#ifdef HIGHFIVE_XTENSOR
template <class T, size_t rank>
inline DataSet dump(File& file,
                    const std::string& path,
                    const xt::xtensor<T, rank>& data,
                    DumpMode mode = DumpMode::Create);
#endif

///
/// Write "std::vector<T>" to a new DataSet in an open HDF5 file.
///
/// @param file opened File (has to be writeable)
/// @param path path of the DataSet
/// @param data the data to write
/// @param mode write mode (DumpMode::Create | DumpMode::Overwrite)
///
/// @return dataset the newly created DataSet (e.g. to add an
/// attribute)
///
template <class T>
inline DataSet dump(File& file,
                    const std::string& path,
                    const std::vector<T>& data,
                    DumpMode mode = DumpMode::Create);

///
/// Write scalar/string to a new DataSet in an open HDF5 file.
///
/// @param file opened File (has to be writeable)
/// @param path path of the DataSet
/// @param data the data to write
/// @param mode write mode (DumpMode::Create | DumpMode::Overwrite)
///
/// @return dataset the newly created DataSet (e.g. to add an
/// attribute)
///
template <class T>
inline DataSet dump(File& file,
                    const std::string& path,
                    const T& data,
                    DumpMode mode = DumpMode::Create);

///
/// Write a scalar to a (new, extendible) DataSet in an open HDF5 file.
///
/// @param file opened File (has to be writeable)
/// @param path path of the DataSet
/// @param data the data to write
/// @param idx the indices to which to write
///
/// @return dataset the (newly created) DataSet (e.g. to add an
/// attribute)
///
template <class T>
inline DataSet dump(File& file,
                    const std::string& path,
                    const T& data,
                    const std::vector<size_t>& idx);

///
/// Load entry "(i,j)" from a rank-two DataSet in an open HDF5 file to a scalar.
///
/// @param file opened File (has to be writeable)
/// @param idx the indices to load
/// @param path path of the DataSet
///
/// @return data the read data
///
template <class T>
inline T load(const File& file,
              const std::string& path,
              const std::vector<size_t>& idx);

///
/// Load a DataSet in an open HDF5 file to an object (templated).
///
/// @param file opened File (has to be writeable)
/// @param path path of the DataSet
///
/// @return data the read data
///
template <class T>
inline T load(const File& file, const std::string& path);

}  // namespace H5Easy

#include "bits/H5Easy_misc.hpp"
#include "bits/H5Easy_scalar.hpp"
#include "bits/H5Easy_vector.hpp"
#include "bits/H5Easy_Eigen.hpp"
#include "bits/H5Easy_xtensor.hpp"

#endif  // H5EASY_HPP
