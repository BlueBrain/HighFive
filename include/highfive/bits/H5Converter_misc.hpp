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

    hdf5_type* getPointer() const {
        return ptr;
    }

    hdf5_type* begin() const {
        return getPointer();
    }

    void unserialize(T& /* val */) const {
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

    hdf5_type* getPointer() {
        return buffer.data();
    }

    hdf5_type const* getPointer() const {
        return buffer.data();
    }

    hdf5_type* begin() {
        return getPointer();
    }

    hdf5_type const* begin() const {
        return getPointer();
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
    explicit Writer(const T& val, const DataType& /* file_datatype */)
        : super(val){};
};

template <typename T>
struct Writer<T, typename enable_deep_copy<T>::type>: public DeepCopyBuffer<T> {
    explicit Writer(const T& val, const DataType& /* file_datatype */)
        : DeepCopyBuffer<T>(inspector<T>::getDimensions(val)) {
        inspector<T>::serialize(val, this->begin());
    }
};

template <typename T, typename Enable = void>
struct Reader;

template <typename T>
struct Reader<T, typename enable_shallow_copy<T>::type>: public ShallowCopyBuffer<T, false> {
  private:
    using super = ShallowCopyBuffer<T, false>;
    using type = typename super::type;

  public:
    Reader(const std::vector<size_t>&, type& val, const DataType& /* file_datatype */)
        : super(val) {}
};

template <typename T>
struct Reader<T, typename enable_deep_copy<T>::type>: public DeepCopyBuffer<T> {
  private:
    using super = DeepCopyBuffer<T>;
    using type = typename super::type;

  public:
    Reader(const std::vector<size_t>& _dims, type&, const DataType& /* file_datatype */)
        : super(_dims) {}
};


struct data_converter {
    template <typename T>
    static Writer<T> serialize(const typename inspector<T>::type& val,
                               const DataType& file_datatype) {
        return Writer<T>(val, file_datatype);
    }

    template <typename T>
    static Reader<T> get_reader(const std::vector<size_t>& dims,
                                T& val,
                                const DataType& file_datatype) {
        // TODO Use bufferinfo for recursive_ndim
        auto effective_dims = details::squeezeDimensions(dims, inspector<T>::recursive_ndim);
        inspector<T>::prepare(val, effective_dims);
        return Reader<T>(effective_dims, val, file_datatype);
    }
};

}  // namespace details
}  // namespace HighFive
