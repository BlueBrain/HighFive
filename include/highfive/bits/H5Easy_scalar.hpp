/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5EASY_BITS_SCALAR_HPP
#define H5EASY_BITS_SCALAR_HPP

#include "../H5Easy.hpp"
#include "H5Easy_misc.hpp"

namespace HighFive {

namespace detail {

namespace scalar {

// create DataSet and write data
template <class T, class E = void>
struct dump_impl
{
    static HighFive::DataSet run(HighFive::File& file, const std::string& path,
        const T& data)
    {
        detail::createGroupsToDataSet(file, path);
        HighFive::DataSet dataset = file.createDataSet<T>(path, HighFive::DataSpace::From(data));
        dataset.write(data);
        file.flush();
        return dataset;
    }
};

// replace data of an existing DataSet of the correct size
template <class T, class E = void>
struct overwrite_impl
{
    static HighFive::DataSet run(HighFive::File& file, const std::string& path,
        const T& data)
    {
        HighFive::DataSet dataset = file.getDataSet(path);
        if (dataset.getElementCount() != 1) {
            throw detail::error(file, path, "HighFive::dump: Existing field not a scalar");
        }
        dataset.write(data);
        file.flush();
        return dataset;
    }
};

// create/write extendible DataSet and write data
template <class T, class E = void>
struct dump_extend_impl
{
    static HighFive::DataSet run(HighFive::File& file, const std::string& path,
        const T& data, const std::vector<size_t>& idx)
    {
        std::vector<size_t> ones(idx.size(), 1);
        if (file.exist(path)) {
            HighFive::DataSet dataset = file.getDataSet(path);
            std::vector<size_t> dims = dataset.getDimensions();
            std::vector<size_t> shape = dims;
            if (dims.size() != idx.size()) {
                throw detail::error(file, path,
                    "HighFive::dump: Rank of the index and the existing field do not match");
            }
            for (size_t i = 0; i < dims.size(); ++i) {
                shape[i] = std::max(dims[i], idx[i] + 1);
            }
            if (shape != dims) {
                dataset.resize(shape);
            }
            dataset.select(idx, ones).write(data);
            file.flush();
            return dataset;
        }
        detail::createGroupsToDataSet(file, path);
        std::vector<size_t> shape = idx;
        const size_t unlim = HighFive::DataSpace::UNLIMITED;
        std::vector<size_t> unlim_shape(idx.size(), unlim);
        std::vector<hsize_t> chuncks(idx.size(), 10);
        for (size_t& i : shape) {
            i++;
        }
        HighFive::DataSpace dataspace = HighFive::DataSpace(shape, unlim_shape);
        HighFive::DataSetCreateProps props;
        props.add(HighFive::Chunking(chuncks));
        HighFive::DataSet dataset = file.createDataSet(path, dataspace, HighFive::AtomicType<T>(), props);
        dataset.select(idx, ones).write(data);
        file.flush();
        return dataset;
    }
};

// load "scalar" as part of a larger DataSet
template <class T>
struct load_impl
{
    static T run(const HighFive::File& file, const std::string& path, const std::vector<size_t>& idx)
    {
        std::vector<size_t> ones(idx.size(), 1);
        HighFive::DataSet dataset = file.getDataSet(path);
        T data;
        dataset.select(idx, ones).read(data);
        return data;
    }
};

}  // namespace scalar

// load "scalar" from DataSet
template <class T, class E = void>
struct load_impl
{
    static T run(const HighFive::File& file, const std::string& path)
    {
        HighFive::DataSet dataset = file.getDataSet(path);
        if (dataset.getElementCount() != 1) {
            throw detail::error(file, path, "HighFive::load: Field not a scalar");
        }
        T data;
        dataset.read(data);
        return data;
    }
};

}  // namespace detail

// front-end
template <class T>
inline HighFive::DataSet dump(HighFive::File& file, const std::string& path,
    const T& data, HighFive::Mode mode)
{
    if (!file.exist(path)) {
        return detail::scalar::dump_impl<T>::run(file, path, data);
    } else if (mode == HighFive::Mode::Overwrite) {
        return detail::scalar::overwrite_impl<T>::run(file, path, data);
    } else {
        throw detail::error(file, path, "path already exists");
    }
}

// front-end
template <class T>
inline HighFive::DataSet dump(HighFive::File& file, const std::string& path,
    const T& data, const std::vector<size_t>& idx)
{
    return detail::scalar::dump_extend_impl<T>::run(file, path, data, idx);
}

// front-end
template <class T>
inline T load(const HighFive::File& file, const std::string& path,
    const std::vector<size_t>& idx)
{
    return detail::scalar::load_impl<T>::run(file, path, idx);
}

// front-end
template <class T>
inline T load(const HighFive::File& file, const std::string& path)
{
    return detail::load_impl<T>::run(file, path);
}

}  // namespace HighFive

#endif  // H5EASY_BITS_SCALAR_HPP
