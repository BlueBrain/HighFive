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

template <class T>
struct is_xtensor : std::false_type {};
template <class T>
struct is_xtensor<xt::xarray<T>> : std::true_type {};
template <class T, size_t N>
struct is_xtensor<xt::xtensor<T, N>> : std::true_type {};

template <typename T>
struct io_impl<T, typename std::enable_if<is_xtensor<T>::value>::type> {

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

    inline static T load(const File& file, const std::string& path) {
        DataSet dataset = file.getDataSet(path);
        std::vector<size_t> dims = dataset.getDimensions();
        T data = T::from_shape(dims);
        dataset.read(data.data());
        return data;
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
