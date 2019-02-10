/*
 *  Copyright (c), 2019, Tom de Geus <tom@geus.me>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5EASY_BITS_EIGEN_HPP
#define H5EASY_BITS_EIGEN_HPP

#include "../H5Easy.hpp"
#include "H5Easy_misc.hpp"
#include "H5Easy_scalar.hpp"  // to get the basic "load_impl"

#ifdef HIGHFIVE_EIGEN

namespace H5Easy {

namespace detail {

namespace eigen {

// return the shape of the "Eigen::Matrix" as size 1 or 2 "std::vector<size_t>"
template <class C, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
std::vector<size_t> shape(const Eigen::Matrix<C,Rows,Cols,Options,MaxRows,MaxCols>& data)
{
    if (Rows == 1) {
        return {static_cast<size_t>(data.cols())};
    } else if (Cols == 1) {
        return {static_cast<size_t>(data.rows())};
    } else {
        return {static_cast<size_t>(data.rows()), static_cast<size_t>(data.cols())};
    }
}

// write to open DataSet of the correct size
// if the "Eigen::Matrix" is column-major reordering has to be done first;
// HDF5 is always row-major
template <class C, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
void write(DataSet& dataset,
           const Eigen::Matrix<C,Rows,Cols,Options,MaxRows,MaxCols>& data)
{
    if (data.IsRowMajor) {
        dataset.write(data.data());
    } else {
        Eigen::Matrix<C, Rows, Cols, Eigen::RowMajor, MaxRows, MaxCols> tmp = data;
        detail::eigen::write(dataset, tmp);
    }
}

// create DataSet and write data
template <class T>
struct dump_impl
{
    template <class C, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
    static DataSet run(File& file,
                       const std::string& path,
                       const Eigen::Matrix<C,Rows,Cols,Options,MaxRows,MaxCols>& data)
    {
        detail::createGroupsToDataSet(file, path);
        DataSet dataset =
            file.createDataSet<C>(path, DataSpace(shape(data)));
        detail::eigen::write(dataset, data);
        file.flush();
        return dataset;
    }
};

// replace data of an existing DataSet of the correct size
template <class T>
struct overwrite_impl
{
    template <class C, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
    static DataSet run(File& file,
                       const std::string& path,
                       const Eigen::Matrix<C,Rows,Cols,Options,MaxRows,MaxCols>& data)
    {
        DataSet dataset = file.getDataSet(path);
        if (dataset.getDimensions() != shape(data)) {
            throw detail::error(file, path, "H5Easy::dump: Inconsistent dimensions");
        }
        detail::eigen::write(dataset, data);
        file.flush();
        return dataset;
    }
};

// load "Eigen::Matrix" from DataSet
// if the "Eigen::Matrix" is column-major reordering has to be done first;
// HDF5 is always row-major
template <class T>
struct load_impl
{
    static T run(const File& file, const std::string& path)
    {
        DataSet dataset = file.getDataSet(path);
        std::vector<size_t> dims = dataset.getDimensions();

        T data;
        if (dims.size() == 1 && T::RowsAtCompileTime == 1) {
            data.resize(1, dims[0]);
        } else if (dims.size() == 1) {
            data.resize(dims[0], 1);
        } else if (dims.size() == 2) {
            data.resize(dims[0], dims[1]);
        } else {
            throw detail::error(file, path, "H5Easy::load: Inconsistent rank");
        }

        if (data.IsRowMajor || T::RowsAtCompileTime == 1 || T::ColsAtCompileTime == 1) {
            dataset.read(data.data());
            return data;
        }

        std::vector<typename T::Scalar> tmp(data.size());
        dataset.read(tmp.data());
        for (int i = 0; i < data.rows(); ++i) {
            for (int j = 0; j < data.cols(); ++j) {
                data(i, j) = tmp[i * data.cols() + j];
            }
        }
        return data;
    }
};

}  // namespace eigen

// front-end
template <class T, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
struct load_impl<Eigen::Matrix<T,Rows,Cols,Options,MaxRows,MaxCols>>
{
    static Eigen::Matrix<T,Rows,Cols,Options,MaxRows,MaxCols>
    run(const File& file,
        const std::string& path)
    {
        return detail::eigen::load_impl<
            Eigen::Matrix<T,Rows,Cols,Options,MaxRows,MaxCols>>::run(file, path);
    }
};

}  // namespace detail

// front-end
template <class T, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
inline DataSet dump(File& file,
                    const std::string& path,
                    const Eigen::Matrix<T,Rows,Cols,Options,MaxRows,MaxCols>& data,
                    DumpMode mode)
{
    if (!file.exist(path)) {
        return detail::eigen::dump_impl<
            Eigen::Matrix<T,Rows,Cols,Options,MaxRows,MaxCols>>::run(file, path, data);
    } else if (mode == DumpMode::Overwrite) {
        return detail::eigen::overwrite_impl<
            Eigen::Matrix<T,Rows,Cols,Options,MaxRows,MaxCols>>::run(file, path, data);
    } else {
        throw detail::error(file, path, "H5Easy: path already exists");
    }
}

}  // namespace H5Easy

#endif  // HIGHFIVE_EIGEN

#endif  // H5EASY_BITS_EIGEN_HPP
