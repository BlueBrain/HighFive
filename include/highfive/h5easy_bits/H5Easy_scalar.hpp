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

namespace H5Easy {

namespace detail {

namespace scalar {

// create DataSet and write data
template <class T, class E = void>
struct dump_impl
{
    static DataSet run(File& file, const std::string& path, const T& data)
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
    static DataSet run(File& file, const std::string& path, const T& data)
    {
        DataSet dataset = file.getDataSet(path);
        if (dataset.getElementCount() != 1) {
            throw detail::error(file, path, "H5Easy::dump: Existing field not a scalar");
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
    static DataSet run(File& file,
                       const std::string& path,
                       const T& data,
                       const std::vector<size_t>& idx)
    {
        std::vector<size_t> ones(idx.size(), 1);

        if (file.exist(path)) {
            DataSet dataset = file.getDataSet(path);
            std::vector<size_t> dims = dataset.getDimensions();
            std::vector<size_t> shape = dims;
            if (dims.size() != idx.size()) {
                throw detail::error(file, path,
                    "H5Easy::dump: Rank of the index and the existing field do not match");
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
        const size_t unlim = DataSpace::UNLIMITED;
        std::vector<size_t> unlim_shape(idx.size(), unlim);
        std::vector<hsize_t> chuncks(idx.size(), 10);
        for (size_t& i : shape) {
            i++;
        }
        DataSpace dataspace = DataSpace(shape, unlim_shape);
        DataSetCreateProps props;
        props.add(Chunking(chuncks));
        DataSet dataset = file.createDataSet(path, dataspace, AtomicType<T>(), props);
        dataset.select(idx, ones).write(data);
        file.flush();
        return dataset;
    }
};

// load "scalar" as part of a larger DataSet
template <class T>
struct load_impl
{
    static T run(const File& file,
                 const std::string& path,
                 const std::vector<size_t>& idx)
    {
        std::vector<size_t> ones(idx.size(), 1);
        DataSet dataset = file.getDataSet(path);
        T data;
        dataset.select(idx, ones).read(data);
        return data;
    }
};

// create Attribute and write data
template <class T, class E = void>
struct dump_attr_impl
{
    static Attribute run(DataSet& dataset, const std::string& key, const T& data)
    {
        Attribute attribute = dataset.createAttribute<T>(key, DataSpace::From(data));
        attribute.write(data);
        return attribute;
    }
};

// replace data of an existing Attribute of the correct size
template <class T, class E = void>
struct overwrite_attr_impl
{
    static Attribute run(DataSet& dataset, const std::string& key, const T& data)
    {
        Attribute attribute = dataset.getAttribute(key);
        DataSpace dataspace = attribute.getSpace();
        if (dataspace.getElementCount() != 1) {
            throw detail::error(key, "Existing field not a scalar");
        }
        attribute.write(data);
        return attribute;
    }
};

}  // namespace scalar

// load from DataSet
template <class T, class E = void>
struct load_impl
{
    static T run(const File& file, const std::string& path)
    {
        DataSet dataset = file.getDataSet(path);
        T data;
        dataset.read(data);
        return data;
    }
};

// load from Attribute
template <class T, class E = void>
struct load_attr_impl
{
    static T run(DataSet& dataset, const std::string& key)
    {
        Attribute attribute = dataset.getAttribute(key);
        T data;
        attribute.read(data);
        return data;
    }
};

}  // namespace detail

// front-end
template <class T>
inline DataSet dump(File& file, const std::string& path, const T& data, DumpMode mode)
{
    if (!file.exist(path)) {
        return detail::scalar::dump_impl<T>::run(file, path, data);
    } else if (mode == DumpMode::Overwrite) {
        return detail::scalar::overwrite_impl<T>::run(file, path, data);
    } else {
        throw detail::error(file, path, "H5Easy: path already exists");
    }
}

// front-end
template <class T>
inline DataSet dump(File& file,
                    const std::string& path,
                    const T& data,
                    const std::vector<size_t>& idx)
{
    return detail::scalar::dump_extend_impl<T>::run(file, path, data, idx);
}

// front-end
template <class T>
inline T load(const File& file, const std::string& path, const std::vector<size_t>& idx)
{
    return detail::scalar::load_impl<T>::run(file, path, idx);
}

// front-end
template <class T>
inline T load(const File& file, const std::string& path)
{
    return detail::load_impl<T>::run(file, path);
}

// front-end
template <class T>
inline Attribute dump_attribute(DataSet& dataset,
                                const std::string& key,
                                const T& data,
                                DumpMode mode)
{
    if (!dataset.hasAttribute(key)) {
        return detail::scalar::dump_attr_impl<T>::run(dataset, key, data);
    } else if (mode == DumpMode::Overwrite) {
        return detail::scalar::overwrite_attr_impl<T>::run(dataset, key, data);
    } else {
        throw detail::error(key, "Attribute already exists");
    }
}

// front-end
template <class T>
inline T load_attribute(DataSet& dataset, const std::string& key)
{
    return detail::load_attr_impl<T>::run(dataset, key);
}

// front-end
template <class T>
inline T load_attribute(const File& file, const std::string& path, const std::string& key)
{
    DataSet dataset = file.getDataSet(path);
    return load_attribute<T>(dataset, key);
}

// front-end
template <class T>
inline Attribute dump_attribute(File& file,
                                const std::string& path,
                                const std::string& key,
                                const T& data,
                                DumpMode mode)
{
    DataSet dataset = file.getDataSet(path);
    Attribute attribute = dump_attribute(dataset, key, data, mode);
    file.flush();
    return attribute;
}

}  // namespace H5Easy

#endif  // H5EASY_BITS_SCALAR_HPP
