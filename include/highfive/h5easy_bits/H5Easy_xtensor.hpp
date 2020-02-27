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
#include "H5Easy_scalar.hpp"  // to get the basic "load_impl"

#ifdef H5_USE_XTENSOR

namespace H5Easy {

namespace detail {

namespace xtensor {

// return the shape of the xtensor-object as "std::vector<size_t>"
template <class T>
inline std::vector<size_t> shape(const T& data)
{
    return std::vector<size_t>(data.shape().cbegin(), data.shape().cend());
}

// create DataSet and write data
template <class T>
static DataSet dump_impl(File& file, const std::string& path, const T& data)
{
    using value_type = typename std::decay_t<T>::value_type;
    detail::createGroupsToDataSet(file, path);
    DataSet dataset = file.createDataSet<value_type>(path, DataSpace(shape(data)));
    dataset.write_raw(data.data());
    file.flush();
    return dataset;
}

// replace data of an existing DataSet of the correct size
template <class T>
static DataSet overwrite_impl(File& file, const std::string& path, const T& data)
{
    DataSet dataset = file.getDataSet(path);
    if (dataset.getDimensions() != shape(data)) {
        throw detail::error(file, path, "H5Easy::dump: Inconsistent dimensions");
    }
    dataset.write_raw(data.data());
    file.flush();
    return dataset;
}

// load xtensor-object from DataSet
template <class T>
struct load_impl
{
    static T run(const File& file, const std::string& path)
    {
        DataSet dataset = file.getDataSet(path);
        std::vector<size_t> dims = dataset.getDimensions();
        T data = T::from_shape(dims);
        dataset.read(data.data());
        return data;
    }
};

// universal front-end (to minimise double code)
template <class T>
inline DataSet dump(File& file,
                    const std::string& path,
                    const T& data,
                    DumpMode mode)
{
    if (!file.exist(path)) {
        return detail::xtensor::dump_impl(file, path, data);
    } else if (mode == DumpMode::Overwrite) {
        return detail::xtensor::overwrite_impl(file, path, data);
    } else {
        throw detail::error(file, path, "H5Easy: path already exists");
    }
}

}  // namespace xtensor

// front-end
template <class T>
struct load_impl<xt::xarray<T>>
{
    static xt::xarray<T> run(const File& file, const std::string& path)
    {
        return detail::xtensor::load_impl<xt::xarray<T>>::run(file, path);
    }
};

// front-end
template <class T, size_t rank>
struct load_impl<xt::xtensor<T, rank>>
{
    static xt::xtensor<T, rank> run(const File& file, const std::string& path)
    {
        return detail::xtensor::load_impl<xt::xtensor<T, rank>>::run(file, path);
    }
};

}  // namespace detail

// front-end
template <class T>
inline DataSet dump(File& file,
                    const std::string& path,
                    const xt::xarray<T>& data,
                    DumpMode mode)
{
    return detail::xtensor::dump(file, path, data, mode);
}

// front-end
template <class T, size_t rank>
inline DataSet dump(File& file,
                    const std::string& path,
                    const xt::xtensor<T, rank>& data,
                    DumpMode mode)
{
    return detail::xtensor::dump(file, path, data, mode);
}

}  // namespace H5Easy

#endif  // H5_USE_XTENSOR

#endif  // H5EASY_BITS_XTENSOR_HPP
