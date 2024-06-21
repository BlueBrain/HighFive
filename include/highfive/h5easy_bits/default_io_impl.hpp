/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include "../H5Easy.hpp"
#include "H5Easy_misc.hpp"
#include "H5Easy_scalar.hpp"

namespace H5Easy {
namespace detail {

using HighFive::details::inspector;

template <typename T>
struct default_io_impl {
    inline static std::vector<size_t> shape(const T& data) {
        return inspector<T>::getDimensions(data);
    }

    inline static DataSet dump(File& file,
                               const std::string& path,
                               const T& data,
                               const DumpOptions& options) {
        using value_type = typename inspector<T>::base_type;
        DataSet dataset = initDataset<value_type>(file, path, shape(data), options);
        dataset.write(data);
        if (options.flush()) {
            file.flush();
        }
        return dataset;
    }

    inline static T load(const File& file, const std::string& path) {
        return file.getDataSet(path).read<T>();
    }

    inline static Attribute dumpAttribute(File& file,
                                          const std::string& path,
                                          const std::string& key,
                                          const T& data,
                                          const DumpOptions& options) {
        using value_type = typename inspector<T>::base_type;
        Attribute attribute = initAttribute<value_type>(file, path, key, shape(data), options);
        attribute.write(data);
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
        return attribute.read<T>();
    }
};

}  // namespace detail
}  // namespace H5Easy
