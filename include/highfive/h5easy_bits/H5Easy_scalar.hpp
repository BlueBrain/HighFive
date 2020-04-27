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

/*
Base template for partial specialization: the fallback if specialized templates don't match.
Used e.g. for scalars.
*/
template <typename T, typename = void>
struct io_impl {

    static DataSet dump(File& file, const std::string& path, const T& data) {
        detail::createGroupsToDataSet(file, path);
        DataSet dataset = file.createDataSet<T>(path, DataSpace::From(data));
        dataset.write(data);
        file.flush();
        return dataset;
    }

    static DataSet overwrite(File& file, const std::string& path, const T& data) {
        DataSet dataset = file.getDataSet(path);
        if (dataset.getElementCount() != 1) {
            throw detail::error(file, path, "H5Easy::dump: Existing field not a scalar");
        }
        dataset.write(data);
        file.flush();
        return dataset;
    }

    static DataSet dump_extend(File& file,
                               const std::string& path,
                               const T& data,
                               const std::vector<size_t>& idx) {
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

    static T load_part(const File& file,
                       const std::string& path,
                       const std::vector<size_t>& idx) {
        std::vector<size_t> ones(idx.size(), 1);
        DataSet dataset = file.getDataSet(path);
        T data;
        dataset.select(idx, ones).read(data);
        return data;
    }

    static T load(const File& file, const std::string& path) {
        DataSet dataset = file.getDataSet(path);
        T data;
        dataset.read(data);
        return data;
    }
};

}  // namespace detail

/*
Frontend functions: dispatch to io_impl<T> and are common for all datatypes.
*/

// front-end
template <class T>
inline DataSet dump(File& file, const std::string& path, const T& data, DumpMode mode) {
    if (!file.exist(path)) {
        return detail::io_impl<T>::dump(file, path, data);
    } else if (mode == DumpMode::Overwrite && file.getObjectType(path) == ObjectType::Dataset) {
        return detail::io_impl<T>::overwrite(file, path, data);
    } else if (file.getObjectType(path) == ObjectType::Dataset) {
        throw detail::error(file, path,
            "H5Easy: Dataset already exists, dump with H5Easy::DumpMode::Overwrite "
            "to overwrite (with an array of the same shape).");
    } else {
        throw detail::error(file, path,
            "H5Easy: path exists, but does not correspond to a Dataset. Dump not possible.");
    }
}

// front-end
template <class T>
inline DataSet dump(File& file,
                    const std::string& path,
                    const T& data,
                    const std::vector<size_t>& idx) {
    return detail::io_impl<T>::dump_extend(file, path, data, idx);
}

// front-end
template <class T>
inline T load(const File& file, const std::string& path, const std::vector<size_t>& idx) {
    return detail::io_impl<T>::load_part(file, path, idx);
}

// front-end
template <class T>
inline T load(const File& file, const std::string& path) {
    return detail::io_impl<T>::load(file, path);
}

}  // namespace H5Easy

#endif  // H5EASY_BITS_SCALAR_HPP
