/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
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

#include "H5File.hpp"
#include "H5DataSet.hpp"
#include "H5DataType.hpp"
#include "H5DataSpace.hpp"

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

namespace HighFive
{
    enum Mode
    {
      Create,
      Overwrite
    };

    ///
    /// Get the groupName of a path to a DataSet. For example for ``path = "/path/to/dataset"`` this
    /// function returns ``"/path/to"``.
    ///
    /// @param path path to a DataSet
    ///
    /// @return group the path of the group above the DataSet
    inline std::string groupName(const std::string& path);

    ///
    /// Recursively create groups in an open HDF5 file such that a DataSet can be created.
    /// For example for ``path = "/path/to/dataset"``, this function will create the groups
    /// ``"/path"`` and ``"/path/to"``.
    ///
    /// @param file opened HighFive::File
    /// @param path path of the DataSet
    ///
    inline void createGroupsToDataSet(HighFive::File& file, const std::string& path);

    ///
    /// Get the size of an existing DataSet in an open HDF5 file.
    ///
    /// @param file opened HighFive::File
    /// @param path path of the DataSet
    ///
    /// @return size the size of the HighFive::DataSet
    inline auto size(const HighFive::File& file, const std::string& path);

    ///
    /// Get the size of an existing DataSet.
    ///
    /// @param dataset opened HighFive::DataSet
    ///
    /// @return size the size of the HighFive::DataSet
    inline auto size(const HighFive::DataSet& dataset);

    ///
    /// Get the shape of an existing DataSet in an open HDF5 file.
    ///
    /// @param file opened HighFive::File
    /// @param path path of the DataSet
    ///
    /// @return shape the shape of the HighFive::DataSet
    inline auto shape(const HighFive::File& file, const std::string& path);

    ///
    /// Get the shape of an existing DataSet.
    ///
    /// @param path path of the DataSet
    ///
    /// @return shape the shape of the HighFive::DataSet
    inline auto shape(const HighFive::DataSet& dataset);

    ///
    /// Write "Eigen::Matrix<T,Rows,Cols,Options>" to a new DataSet in an open HDF5 file.
    ///
    /// @param file opened HighFive::File (has to be writeable)
    /// @param path path of the DataSet
    /// @param data the data to write
    /// @param mode write mode (HighFive::Mode::Create | HighFive::Mode::Overwrite)
    ///
    /// @return dataset the newly created HighFive::DataSet (e.g. to add an attribute)
    ///
    #ifdef HIGHFIVE_EIGEN
    template <class T, int Rows, int Cols, int Options>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const Eigen::Matrix<T,Rows,Cols,Options>& data, HighFive::Mode mode=HighFive::Mode::Create);
    #endif

    ///
    /// Write "xt::xarray<T>" to a new DataSet in an open HDF5 file.
    ///
    /// @param file opened HighFive::File (has to be writeable)
    /// @param path path of the DataSet
    /// @param data the data to write
    /// @param mode write mode (HighFive::Mode::Create | HighFive::Mode::Overwrite)
    ///
    /// @return dataset the newly created HighFive::DataSet (e.g. to add an attribute)
    ///
    #ifdef HIGHFIVE_XTENSOR
    template <class T>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const xt::xarray<T>& data, HighFive::Mode mode=HighFive::Mode::Create);
    #endif

    ///
    /// Write "xt::xtensor<T,rank>" to a new DataSet in an open HDF5 file.
    ///
    /// @param file opened HighFive::File (has to be writeable)
    /// @param path path of the DataSet
    /// @param data the data to write
    /// @param mode write mode (HighFive::Mode::Create | HighFive::Mode::Overwrite)
    ///
    /// @return dataset the newly created HighFive::DataSet (e.g. to add an attribute)
    ///
    #ifdef HIGHFIVE_XTENSOR
    template <class T, std::size_t rank>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const xt::xtensor<T,rank>& data, HighFive::Mode mode=HighFive::Mode::Create);
    #endif

    ///
    /// Write "std::vector<T>" to a new DataSet in an open HDF5 file.
    ///
    /// @param file opened HighFive::File (has to be writeable)
    /// @param path path of the DataSet
    /// @param data the data to write
    /// @param mode write mode (HighFive::Mode::Create | HighFive::Mode::Overwrite)
    ///
    /// @return dataset the newly created HighFive::DataSet (e.g. to add an attribute)
    ///
    template <class T>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const std::vector<T>& data, HighFive::Mode mode=HighFive::Mode::Create);

    ///
    /// Write scalar/string to a new DataSet in an open HDF5 file.
    ///
    /// @param file opened HighFive::File (has to be writeable)
    /// @param path path of the DataSet
    /// @param data the data to write
    /// @param mode write mode (HighFive::Mode::Create | HighFive::Mode::Overwrite)
    ///
    /// @return dataset the newly created HighFive::DataSet (e.g. to add an attribute)
    ///
    template <class T>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const T& data, HighFive::Mode mode=HighFive::Mode::Create);

    ///
    /// Write a scalar to a (new, extendible) DataSet in an open HDF5 file.
    ///
    /// @param file opened HighFive::File (has to be writeable)
    /// @param path path of the DataSet
    /// @param data the data to write
    /// @param idx the indices to which to write
    ///
    /// @return dataset the (newly created) HighFive::DataSet (e.g. to add an attribute)
    ///
    template <class T>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const T& data, const std::vector<std::size_t>& idx);

    ///
    /// Load entry "(i,j)" from a rank-two DataSet in an open HDF5 file to a scalar.
    ///
    /// @param file opened HighFive::File (has to be writeable)
    /// @param idx the indices to load
    /// @param path path of the DataSet
    ///
    /// @return data the read data
    ///
    template <class T>
    inline auto load(const HighFive::File& file, const std::string& path, const std::vector<std::size_t>& idx);

    ///
    /// Load a DataSet in an open HDF5 file to an object (templated).
    ///
    /// @param file opened HighFive::File (has to be writeable)
    /// @param path path of the DataSet
    ///
    /// @return data the read data
    ///
    template <class T>
    inline auto load(const HighFive::File& file, const std::string& path);

}

#include "bits/H5Easy_misc.hpp"
#include "bits/H5Easy_scalar.hpp"
#include "bits/H5Easy_vector.hpp"
#include "bits/H5Easy_Eigen.hpp"
#include "bits/H5Easy_xtensor.hpp"

#endif // H5EASY_HPP

