/*
 *  Copyright (c) 2022 Blue Brain Project
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <type_traits>

#include "H5Inspector_misc.hpp"
#include "../H5DataType.hpp"

namespace HighFive {
namespace details {

template <class T, class V = void>
struct enable_shallow_copy: public std::enable_if<inspector<T>::is_trivially_copyable, V> {};

template <class T, class V = void>
struct enable_deep_copy: public std::enable_if<!inspector<T>::is_trivially_copyable, V> {};

template <typename T, bool IsReadOnly>
struct ShallowCopyBuffer {
    using type = unqualified_t<T>;
    using hdf5_type =
        typename std::conditional<IsReadOnly,
                                  typename std::add_const<typename inspector<T>::hdf5_type>::type,
                                  typename inspector<T>::hdf5_type>::type;

    ShallowCopyBuffer() = delete;

    explicit ShallowCopyBuffer(typename std::conditional<IsReadOnly, const T&, T&>::type val)
        : ptr(inspector<T>::data(val)){};

    hdf5_type* get_pointer() const {
        return ptr;
    }

    void unserialize(T& val) const {
        /* nothing to do. */
    }

  private:
    hdf5_type* ptr;
};

template <class T>
struct DeepCopyBuffer {
    using type = unqualified_t<T>;
    using hdf5_type = typename inspector<type>::hdf5_type;

    explicit DeepCopyBuffer(const std::vector<size_t>& _dims)
        : buffer(inspector<T>::getSize(_dims))
        , dims(_dims) {}

    hdf5_type* get_pointer() {
        return buffer.data();
    }

    hdf5_type const* get_pointer() const {
        return buffer.data();
    }

    void unserialize(T& val) const {
        inspector<type>::unserialize(buffer.data(), dims, val);
    }

  private:
    std::vector<hdf5_type> buffer;
    std::vector<size_t> dims;
};


template <typename T, typename Enable = void>
struct Writer;

template <typename T>
struct Writer<T, typename enable_shallow_copy<T>::type>: public ShallowCopyBuffer<T, true> {
  private:
    using super = ShallowCopyBuffer<T, true>;

  public:
    explicit Writer(const T& val)
        : super(val){};
};

template <typename T>
struct Writer<T, typename enable_deep_copy<T>::type>: public DeepCopyBuffer<T> {
    explicit Writer(const T& val)
        : DeepCopyBuffer<T>(inspector<T>::getDimensions(val)) {
        inspector<T>::serialize(val, this->get_pointer());
    }
};


template <typename T, typename Enable = void>
struct Reader;

template <typename T>
struct Reader<T, typename enable_shallow_copy<T>::type>: ShallowCopyBuffer<T, false> {
  private:
    using super = ShallowCopyBuffer<T, false>;
    using type = typename super::type;

  public:
    Reader(const std::vector<size_t>&, type& val)
        : super(val) {}
};

template <typename T>
struct Reader<T, typename enable_deep_copy<T>::type>: public DeepCopyBuffer<T> {
  private:
    using super = DeepCopyBuffer<T>;
    using type = typename super::type;

  public:
    Reader(const std::vector<size_t>& _dims, type&)
        : super(_dims) {}
};


struct data_converter {
    template <typename T>
    static Writer<T> serialize(const typename inspector<T>::type& val) {
        return Writer<T>(val);
    }

    template <typename T>
    static Reader<T> get_reader(const std::vector<size_t>& dims, T& val) {
        // TODO Use bufferinfo for recursive_ndim
        auto effective_dims = details::squeezeDimensions(dims, inspector<T>::recursive_ndim);
        inspector<T>::prepare(val, effective_dims);
        return Reader<T>(effective_dims, val);
    }
};

}  // namespace details
}  // namespace HighFive
