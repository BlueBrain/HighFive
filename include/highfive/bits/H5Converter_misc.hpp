/*
 *  Copyright (c) 2022 Blue Brain Project
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include "H5Inspector_misc.hpp"

namespace HighFive {
namespace details {

template <typename T>
struct Writer {
    using hdf5_type = typename inspector<T>::hdf5_type;
    const hdf5_type* get_pointer() {
        if (vec.empty()) {
            return ptr;
        } else {
            return vec.data();
        }
    }
    std::vector<hdf5_type> vec{};
    const hdf5_type* ptr{nullptr};
};

template <typename T>
struct Reader {
    using type = unqualified_t<T>;
    using hdf5_type = typename inspector<type>::hdf5_type;

    Reader(const std::vector<size_t>& _dims, type& _val)
        : dims(_dims)
        , val(_val) {}

    hdf5_type* get_pointer() {
        if (vec.empty()) {
            return inspector<type>::data(val);
        } else {
            return vec.data();
        }
    }

    void unserialize() {
        if (!vec.empty()) {
            inspector<type>::unserialize(vec.data(), dims, val);
        }
    }

    std::vector<size_t> dims{};
    std::vector<hdf5_type> vec{};
    type& val{};
};

struct data_converter {
    template <typename T>
    static typename std::enable_if<inspector<T>::is_trivially_copyable, Writer<T>>::type serialize(
        const typename inspector<T>::type& val) {
        Writer<T> w;
        w.ptr = inspector<T>::data(val);
        return w;
    }

    template <typename T>
    static typename std::enable_if<!inspector<T>::is_trivially_copyable, Writer<T>>::type serialize(
        const typename inspector<T>::type& val) {
        Writer<T> w;
        w.vec.resize(inspector<T>::getSizeVal(val));
        inspector<T>::serialize(val, w.vec.data());
        return w;
    }

    template <typename T>
    static
        typename std::enable_if<inspector<unqualified_t<T>>::is_trivially_copyable, Reader<T>>::type
        get_reader(const std::vector<size_t>& dims, T& val) {
        auto effective_dims = details::squeezeDimensions(dims, inspector<T>::recursive_ndim);
        Reader<T> r(effective_dims, val);
        inspector<T>::prepare(r.val, effective_dims);
        return r;
    }

    template <typename T>
    static typename std::enable_if<!inspector<unqualified_t<T>>::is_trivially_copyable,
                                   Reader<T>>::type
    get_reader(const std::vector<size_t>& dims, T& val) {
        auto effective_dims = details::squeezeDimensions(dims, inspector<T>::recursive_ndim);

        Reader<T> r(effective_dims, val);
        inspector<T>::prepare(r.val, effective_dims);
        r.vec.resize(inspector<T>::getSize(effective_dims));
        return r;
    }
};

}  // namespace details
}  // namespace HighFive
