/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5EASY_BITS_VECTOR_HPP
#define H5EASY_BITS_VECTOR_HPP

#include "../H5Easy.hpp"
#include "H5Easy_misc.hpp"
#include "H5Easy_scalar.hpp"  // to get the basic "load_impl"

namespace H5Easy {

namespace detail {

namespace vector {

// create DataSet and write data
template <class T, class E = void>
struct dump_impl
{
    static DataSet run(File& file, const std::string& path, const std::vector<T>& data)
    {
        detail::createGroupsToDataSet(file, path);
        DataSet dataset = file.createDataSet<T>(path, DataSpace::From(data));
        dataset.write(data);
        file.flush();
        return dataset;
    }
};

// replace data of an existing DataSet of the correct size
template <class T, class E = void>
struct overwrite_impl
{
    static DataSet run(File& file, const std::string& path, const std::vector<T>& data)
    {
        DataSet dataset = file.getDataSet(path);
        std::vector<size_t> dims = dataset.getDimensions();
        if (dims.size() > 1) {
            throw detail::error(file, path,
                "H5Easy::dump: Can only overwrite 1-d vectors");
        }
        if (dims[0] != data.size()) {
            throw detail::error(file, path, "H5Easy::dump: Inconsistent dimensions");
        }
        dataset.write(data);
        file.flush();
        return dataset;
    }
};

}  // namespace vector

// load "std::vector" from DataSet
template <class T>
struct load_impl<std::vector<T>>
{
    static std::vector<T> run(const File& file, const std::string& path)
    {
        DataSet dataset = file.getDataSet(path);
        std::vector<size_t> dims = dataset.getDimensions();
        if (dims.size() != 1) {
            throw detail::error(file, path, "H5Easy::load: Field not rank 1");
        }
        std::vector<T> data;
        dataset.read(data);
        return data;
    }
};

}  // namespace detail

// front-end
template <class T>
inline DataSet dump(File& file,
                    const std::string& path,
                    const std::vector<T>& data,
                    DumpMode mode)
{
    if (!file.exist(path)) {
        return detail::vector::dump_impl<T>::run(file, path, data);
    } else if (mode == DumpMode::Overwrite) {
        return detail::vector::overwrite_impl<T>::run(file, path, data);
    } else {
        throw detail::error(file, path, "H5Easy: path already exists");
    }
}

}  // namespace H5Easy

#endif  // H5EASY_BITS_VECTOR_HPP
