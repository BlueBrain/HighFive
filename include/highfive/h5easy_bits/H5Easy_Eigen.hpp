/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
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

#ifdef H5_USE_EIGEN

namespace H5Easy {

namespace detail {

namespace eigen {

// return the shape of the "Eigen::Matrix" as size 1 or 2 "std::vector<size_t>"
template <class T>
inline std::vector<size_t> shape(const T& data) {
    if (std::decay<T>::type::RowsAtCompileTime == 1) {
        return {static_cast<size_t>(data.cols())};
    }
    if (std::decay<T>::type::ColsAtCompileTime == 1) {
        return {static_cast<size_t>(data.rows())};
    }
    return {static_cast<size_t>(data.rows()),
            static_cast<size_t>(data.cols())};
}


using EigenIndex = Eigen::DenseIndex;

// get the shape of a "DataSet" as size 2 "std::vector<Eigen::Index>"
inline std::vector<EigenIndex> shape(const File& file,
                                     const std::string& path,
                                     const DataSet& dataset,
                                     int RowsAtCompileTime) {
    std::vector<size_t> dims = dataset.getDimensions();

    if (dims.size() == 1 && RowsAtCompileTime == 1) {
        return std::vector<EigenIndex>{1u, static_cast<EigenIndex>(dims[0])};
    }
    if (dims.size() == 1) {
        return std::vector<EigenIndex>{static_cast<EigenIndex>(dims[0]), 1u};
    }
    if (dims.size() == 2) {
        return std::vector<EigenIndex>{static_cast<EigenIndex>(dims[0]),
                                       static_cast<EigenIndex>(dims[1])};
    }

    throw detail::error(file, path, "H5Easy::load: Inconsistent rank");
}

// write to open DataSet of the correct size
// (use Eigen::Ref to convert to RowMajor; no action if no conversion is needed)
template <class T>
inline void write(DataSet& dataset, const T& data) {
    Eigen::Ref<
        const Eigen::Matrix<
            typename std::decay<T>::type::Scalar,
            std::decay<T>::type::RowsAtCompileTime,
            std::decay<T>::type::ColsAtCompileTime,
            std::decay<T>::type::ColsAtCompileTime==1 ? Eigen::ColMajor : Eigen::RowMajor,
            std::decay<T>::type::MaxRowsAtCompileTime,
            std::decay<T>::type::MaxColsAtCompileTime>,
        0,
        Eigen::InnerStride<1>> row_major(data);

    dataset.write_raw(row_major.data());
}

// create DataSet and write data
template <class T>
static DataSet dump_impl(File& file, const std::string& path, const T& data) {
    using value_type = typename std::decay<T>::type::Scalar;
    detail::createGroupsToDataSet(file, path);
    DataSet dataset = file.createDataSet<value_type>(path, DataSpace(shape(data)));
    detail::eigen::write(dataset, data);
    file.flush();
    return dataset;
}

// replace data of an existing DataSet of the correct size
template <class T>
static DataSet overwrite_impl(File& file, const std::string& path, const T& data) {
    DataSet dataset = file.getDataSet(path);
    if (dataset.getDimensions() != shape(data)) {
        throw detail::error(file, path, "H5Easy::dump: Inconsistent dimensions");
    }
    detail::eigen::write(dataset, data);
    file.flush();
    return dataset;
}

// load from DataSet
// convert to ColMajor if needed (HDF5 always stores row-major)
template <class T>
struct load_impl {
    static T run(const File& file, const std::string& path) {
        DataSet dataset = file.getDataSet(path);
        std::vector<typename T::Index> dims = shape(file, path, dataset, T::RowsAtCompileTime);
        T data(dims[0], dims[1]);
        dataset.read(data.data());

        if (data.IsVectorAtCompileTime || data.IsRowMajor) {
            return data;
        }

        return Eigen::Map<Eigen::Matrix<
            typename T::Scalar,
            T::RowsAtCompileTime,
            T::ColsAtCompileTime,
            T::ColsAtCompileTime==1?Eigen::ColMajor:Eigen::RowMajor,
            T::MaxRowsAtCompileTime,
            T::MaxColsAtCompileTime>>(data.data(), dims[0], dims[1]);
    }
};

// universal front-end (to minimise double code)
template <class T>
inline DataSet dump(File& file, const std::string& path, const T& data, DumpMode mode) {
    if (!file.exist(path)) {
        return detail::eigen::dump_impl(file, path, data);
    } else if (mode == DumpMode::Overwrite) {
        return detail::eigen::overwrite_impl(file, path, data);
    } else {
        throw detail::error(file, path, "H5Easy: path already exists");
    }
}

}  // namespace eigen

// front-end
template <class T, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
struct load_impl<Eigen::Matrix<T,Rows,Cols,Options,MaxRows,MaxCols>> {
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
                    DumpMode mode) {
    return detail::eigen::dump(file, path, data, mode);
}

// front-end
template <class T>
inline DataSet dump(File& file,
                    const std::string& path,
                    const Eigen::Ref<T>& data,
                    DumpMode mode) {
    return detail::eigen::dump(file, path, data, mode);
}

// front-end
template <class T>
inline DataSet dump(File& file,
                    const std::string& path,
                    const Eigen::Map<T>& data,
                    DumpMode mode) {
    return detail::eigen::dump(file, path, data, mode);
}

}  // namespace H5Easy

#endif  // H5_USE_EIGEN

#endif  // H5EASY_BITS_EIGEN_HPP
