/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5EASY_BITS_OPENCV_HPP
#define H5EASY_BITS_OPENCV_HPP

#include "../H5Easy.hpp"
#include "H5Easy_misc.hpp"
#include "H5Easy_scalar.hpp"

#ifdef H5_USE_OPENCV

namespace H5Easy {

namespace detail {

template <class T>
struct is_opencv : std::false_type {};
template <class T>
struct is_opencv<cv::Mat_<T>> : std::true_type {};

template <typename T>
struct io_impl<T, typename std::enable_if<is_opencv<T>::value>::type> {

    inline static std::vector<size_t> shape(const T& data)
    {
        std::vector<size_t> out(2);
        out[0] = static_cast<size_t>(data.rows);
        out[1] = static_cast<size_t>(data.cols);
        return out;
    }

    inline static std::vector<int> shape(const File& file,
                                         const std::string& path,
                                         const DataSet& dataset)
    {
        std::vector<size_t> dims = dataset.getDimensions();

        if (dims.size() == 1) {
            return std::vector<int>{static_cast<int>(dims[0]), 1ul};
        }
        if (dims.size() == 2) {
            return std::vector<int>{static_cast<int>(dims[0]),
                                    static_cast<int>(dims[1])};
        }

        throw detail::error(file, path, "H5Easy::load: Inconsistent rank");
    }

    inline static DataSet write(File& file, DataSet& dataset, const T& data)
    {
        using value_type = typename T::value_type;
        std::vector<value_type> v (data.begin(), data.end());
        dataset.write_raw(v.data());
        file.flush();
        return dataset;
    }

    inline static DataSet dump(File& file, const std::string& path, const T& data) {
        using value_type = typename T::value_type;
        detail::createGroupsToDataSet(file, path);
        DataSet dataset = file.createDataSet<value_type>(path, DataSpace(shape(data)));
        return write(file, dataset, data);
    }

    inline static DataSet overwrite(File& file, const std::string& path, const T& data) {
        DataSet dataset = file.getDataSet(path);
        if (dataset.getDimensions() != shape(data)) {
            throw detail::error(file, path, "H5Easy::dump: Inconsistent dimensions");
        }
        return write(file, dataset, data);
    }

    inline static T load(const File& file, const std::string& path) {
        using value_type = typename T::value_type;
        DataSet dataset = file.getDataSet(path);
        std::vector<int> dims = shape(file, path, dataset);
        T data(dims[0], dims[1]);
        dataset.read(reinterpret_cast<value_type*>(data.data));
        return data;
    }
};

}  // namespace detail
}  // namespace H5Easy

#endif  // H5_USE_OPENCV

#endif  // H5EASY_BITS_OPENCV_HPP
