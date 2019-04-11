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
std::vector<size_t> shape(const T& data)
{
    return std::vector<size_t>(data.shape().cbegin(), data.shape().cend());
}

// create DataSet and write data
template <class T>
struct dump_impl
{
    template <class C>
    static DataSet run(File& file, const std::string& path, const C& data)
    {
        detail::createGroupsToDataSet(file, path);
        DataSet dataset =
            file.createDataSet<typename C::value_type>(path, DataSpace(shape(data)));
        dataset.write(data.begin());
        file.flush();
        return dataset;
    }
};

// replace data of an existing DataSet of the correct size
template <class T>
struct overwrite_impl
{
    template <class C>
    static DataSet run(File& file, const std::string& path, const C& data)
    {
        DataSet dataset = file.getDataSet(path);
        if (dataset.getDimensions() != shape(data)) {
            throw detail::error(file, path, "H5Easy::dump: Inconsistent dimensions");
        }
        dataset.write(data.begin());
        file.flush();
        return dataset;
    }
};

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
    if (!file.exist(path)) {
        return detail::xtensor::dump_impl<xt::xarray<T>>::run(file, path, data);
    } else if (mode == DumpMode::Overwrite) {
        return detail::xtensor::overwrite_impl<xt::xarray<T>>::run(file, path, data);
    } else {
        throw detail::error(file, path, "H5Easy: path already exists");
    }
}

// front-end
template <class T, size_t rank>
inline DataSet dump(File& file,
                    const std::string& path,
                    const xt::xtensor<T, rank>& data,
                    DumpMode mode)
{
    if (!file.exist(path)) {
        return detail::xtensor::dump_impl<xt::xtensor<T, rank>>::run(file, path, data);
    } else if (mode == DumpMode::Overwrite) {
        return detail::xtensor::overwrite_impl<xt::xtensor<T, rank>>::run(file, path, data);
    } else {
        throw detail::error(file, path, "H5Easy: path already exists");
    }
}

}  // namespace H5Easy

#endif  // H5_USE_XTENSOR

#endif  // H5EASY_BITS_XTENSOR_HPP
