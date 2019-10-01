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

using HighFive::details::type_of_array;
using HighFive::details::get_dim_vector;

// create DataSet and write data
template <class T, class E = void>
struct dump_impl
{
    static DataSet run(File& file, const std::string& path, const std::vector<T>& data)
    {
        using type_name = typename type_of_array<T>::type;
        detail::createGroupsToDataSet(file, path);
        DataSet dataset = file.createDataSet<type_name>(path, DataSpace::From(data));
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
        if (get_dim_vector(data) != dataset.getDimensions())
        {
            throw detail::error(file, path, "H5Easy::dump: Inconsistent dimensions");
        }
        dataset.write(data);
        file.flush();
        return dataset;
    }
};

}  // namespace vector

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
