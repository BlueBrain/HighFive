/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5EASY_BITS_XTENSOR_HPP
#define H5EASY_BITS_XTENSOR_HPP

#include "../H5Easy.hpp"
#include "H5Easy_misc.hpp"
#include "H5Easy_scalar.hpp"

#ifdef H5_USE_XTENSOR

namespace H5Easy {

namespace detail {

template <typename T>
struct io_impl<T, typename std::enable_if<xt::is_xexpression<T>::value>::type> {
    inline static std::vector<size_t> shape(const T& data) {
        return std::vector<size_t>(data.shape().cbegin(), data.shape().cend());
    }

    inline static DataSet dump(File& file,
                               const std::string& path,
                               const T& data,
                               const DumpOptions& options) {
        using value_type = typename std::decay_t<T>::value_type;
        DataSet dataset = initDataset<value_type>(file, path, shape(data), options);
        dataset.write_raw(data.data());
        if (options.flush()) {
            file.flush();
        }
        return dataset;
    }

    inline static DataSet dump_extend(File& file,
                                      const std::string& path,
                                      const T& data,
                                      const std::vector<size_t>& idx,
                                      const DumpOptions& options) {
        using value_type = typename std::decay_t<T>::value_type;
        std::vector<size_t> dataShape = shape(data);

        if (file.exist(path)) {
            DataSet dataset = file.getDataSet(path);

            std::vector<size_t> dims = dataset.getDimensions();
            std::vector<size_t> shape = dims;
            if (dims.size() != idx.size()) {
                throw detail::error(
                    file,
                    path,
                    "H5Easy::dump: Dimension of the index and the existing field do not match");
            }
            for (size_t i = 0; i < dims.size(); ++i) {
                shape[i] = std::max(dims[i], idx[i] + dataShape[i]);
            }
            if (shape != dims) {
                dataset.resize(shape);
            }
            dataset.select(idx, dataShape).write_raw(data.data());
            if (options.flush()) {
                file.flush();
            }
            return dataset;

        }

        std::vector<size_t> shape = idx;
        const size_t unlim = DataSpace::UNLIMITED;
        std::vector<size_t> unlim_shape(idx.size(), unlim);
        std::vector<hsize_t> chunks(idx.size(), dataShape.size());
        if (options.isChunked()) {
            chunks = options.getChunkSize();
            if (chunks.size() != idx.size()) {
                throw error(file, path, "H5Easy::dump: Incorrect dimension ChunkSize");
            }
        }
        for (size_t i = 0; i < shape.size(); ++i) {
            shape[i] += dataShape[i];
        }
        DataSetCreateProps props;
        props.add(Chunking(chunks));
        DataSet dataset = file.createDataSet<value_type>(path, DataSpace(shape), props, {}, true);
        dataset.select(idx, dataShape).write_raw(data.data());
        if (options.flush()) {
            file.flush();
        }
        return dataset;
    }

    inline static T load(const File& file, const std::string& path) {
        static_assert(
            xt::has_data_interface<T>::value,
            "Cannot load to xt::xfunction or xt::xgenerator, use e.g. xt::xtensor or xt::xarray");
        DataSet dataset = file.getDataSet(path);
        std::vector<size_t> dims = dataset.getDimensions();
        T data = T::from_shape(dims);
        dataset.read(data.data());
        return data;
    }

    inline static T load_part(const File& file,
                              const std::string& path,
                              const std::vector<size_t>& idx,
                              const std::vector<size_t>& sizes) {
        static_assert(
            xt::has_data_interface<T>::value,
            "Cannot load to xt::xfunction or xt::xgenerator, use e.g. xt::xtensor or xt::xarray");
        DataSet dataset = file.getDataSet(path);
        std::vector<size_t> dims = dataset.getDimensions();
        std::vector<size_t> shape = sizes;

        for (size_t i = 0; i < dims.size(); ++i) {
            shape[i] = std::min(sizes[i], dims[i] - idx[i]);
        }

        T data = T::from_shape(shape);
        dataset.select(idx, shape).read(data.data());
        return data;
    }

    inline static T load_part(const File& file,
                              const std::string& path,
                              const std::vector<size_t>& idx) {
        return load_part(file, path, idx, std::vector<size_t>(idx.size(), 1));
    }

    inline static Attribute dumpAttribute(File& file,
                                          const std::string& path,
                                          const std::string& key,
                                          const T& data,
                                          const DumpOptions& options) {
        using value_type = typename std::decay_t<T>::value_type;
        Attribute attribute = initAttribute<value_type>(file, path, key, shape(data), options);
        attribute.write_raw(data.data());
        if (options.flush()) {
            file.flush();
        }
        return attribute;
    }

    inline static T loadAttribute(const File& file,
                                  const std::string& path,
                                  const std::string& key) {
        static_assert(
            xt::has_data_interface<T>::value,
            "Cannot load to xt::xfunction or xt::xgenerator, use e.g. xt::xtensor or xt::xarray");
        DataSet dataset = file.getDataSet(path);
        Attribute attribute = dataset.getAttribute(key);
        DataSpace dataspace = attribute.getSpace();
        std::vector<size_t> dims = dataspace.getDimensions();
        T data = T::from_shape(dims);
        attribute.read(data.data());
        return data;
    }
};

}  // namespace detail
}  // namespace H5Easy

#endif  // H5_USE_XTENSOR
#endif  // H5EASY_BITS_XTENSOR_HPP
