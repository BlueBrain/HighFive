/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include "../H5Easy.hpp"
#include "H5Easy_misc.hpp"
#include "H5Easy_scalar.hpp"

#ifdef H5_USE_EIGEN

#include "../eigen.hpp"

namespace H5Easy {

namespace detail {

template <typename T>
struct io_impl<T, typename std::enable_if<std::is_base_of<Eigen::DenseBase<T>, T>::value>::type> {
    using EigenIndex = Eigen::DenseIndex;

    // When creating a dataset for an Eigen object, the shape of the dataset is
    // 1D for vectors. (legacy reasons)
    inline static std::vector<size_t> file_shape(const T& data) {
        if (std::decay<T>::type::RowsAtCompileTime == 1) {
            return {static_cast<size_t>(data.cols())};
        }
        if (std::decay<T>::type::ColsAtCompileTime == 1) {
            return {static_cast<size_t>(data.rows())};
        }
        return inspector<T>::getDimensions(data);
    }

    // The shape of an Eigen object as used in HighFive core.
    inline static std::vector<size_t> mem_shape(const T& data) {
        return inspector<T>::getDimensions(data);
    }

    // The shape of an Eigen object as used in HighFive core.
    template <class D>
    inline static std::vector<size_t> mem_shape(const File& file,
                                                const std::string& path,
                                                const D& dataset) {
        std::vector<size_t> dims = dataset.getDimensions();

        if (dims.size() == 1 && T::RowsAtCompileTime == 1) {
            return std::vector<size_t>{1, dims[0]};
        }
        if (dims.size() == 1 && T::ColsAtCompileTime == 1) {
            return std::vector<size_t>{dims[0], 1};
        }
        if (dims.size() == 2) {
            return dims;
        }

        throw detail::error(file, path, "H5Easy::load: Inconsistent rank");
    }

    inline static DataSet dump(File& file,
                               const std::string& path,
                               const T& data,
                               const DumpOptions& options) {
        using value_type = typename std::decay<T>::type::Scalar;

        std::vector<size_t> file_dims = file_shape(data);
        std::vector<size_t> mem_dims = mem_shape(data);
        DataSet dataset = initDataset<value_type>(file, path, file_dims, options);
        dataset.reshapeMemSpace(mem_dims).write(data);
        if (options.flush()) {
            file.flush();
        }
        return dataset;
    }

    inline static T load(const File& file, const std::string& path) {
        DataSet dataset = file.getDataSet(path);
        std::vector<size_t> dims = mem_shape(file, path, dataset);
        return dataset.reshapeMemSpace(dims).template read<T>();
    }

    inline static Attribute dumpAttribute(File& file,
                                          const std::string& path,
                                          const std::string& key,
                                          const T& data,
                                          const DumpOptions& options) {
        using value_type = typename std::decay<T>::type::Scalar;

        std::vector<size_t> file_dims = file_shape(data);
        std::vector<size_t> mem_dims = mem_shape(data);
        Attribute attribute = initAttribute<value_type>(file, path, key, file_dims, options);
        attribute.reshapeMemSpace(mem_dims).write(data);
        if (options.flush()) {
            file.flush();
        }
        return attribute;
    }

    inline static T loadAttribute(const File& file,
                                  const std::string& path,
                                  const std::string& key) {
        DataSet dataset = file.getDataSet(path);
        Attribute attribute = dataset.getAttribute(key);
        DataSpace dataspace = attribute.getSpace();
        std::vector<size_t> dims = mem_shape(file, path, dataspace);
        return attribute.reshapeMemSpace(dims).template read<T>();
    }
};

}  // namespace detail
}  // namespace H5Easy

#endif  // H5_USE_EIGEN
