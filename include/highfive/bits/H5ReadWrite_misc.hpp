/*
 *  Copyright (c) 2020 Blue Brain Project
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <H5Tpublic.h>
#include "H5Utils.hpp"

namespace HighFive {

namespace details {

template <typename T>
using unqualified_t = typename std::remove_const<typename std::remove_reference<T>::type>::type;

// Find the type of an eventual char array, otherwise void
template <typename T>
struct type_char_array {
    using type = typename std::conditional<
        std::is_same<typename inspector<T>::base_type, std::string>::value,
        std::string,
        void>::type;
    static constexpr bool is_char_array = false;
};

template <typename T>
struct type_char_array<T*> {
    using type = typename std::conditional<std::is_same<unqualified_t<T>, char>::value,
                                           char*,
                                           typename type_char_array<T>::type>::type;
    static constexpr bool is_char_array = true;
};

template <typename T, std::size_t N>
struct type_char_array<T[N]> {
    using type = typename std::conditional<std::is_same<unqualified_t<T>, char>::value,
                                           char[N],
                                           typename type_char_array<T>::type>::type;
    static constexpr bool is_char_array = true;
};

template <typename T>
struct BufferInfo {
    using type_no_const = typename std::remove_const<T>::type;
    using elem_type = typename details::inspector<type_no_const>::base_type;
    using char_array_t = typename details::type_char_array<type_no_const>::type;
    static constexpr bool is_char_array = details::type_char_array<type_no_const>::is_char_array;

    enum Operation { read, write };
    const Operation op;

    template <class F>
    BufferInfo(const DataType& dtype, F getName, Operation _op);

    // member data for info depending on the destination dataset type
    const bool is_fixed_len_string;
    const size_t n_dimensions;
    const DataType data_type;
};

// details implementation
template <typename SrcStrT>
struct string_type_checker {
    static DataType getDataType(const DataType&, const DataType&);
};

inline void enforce_ascii_hack(const DataType& dst, const DataType& src) {
    // Note: constness only refers to constness of the DataType object, which
    // is just an ID, we can/will change properties of `dst`.

    // TEMP. CHANGE: Ensure that the character set is properly configured to prevent
    // converter issues on HDF5 <=v1.12.0 when loading ASCII strings first.
    // See https://github.com/HDFGroup/hdf5/issues/544 for further information.

    bool is_dst_string = detail::h5t_get_class(dst.getId()) == H5T_STRING;
    bool is_src_string = detail::h5t_get_class(src.getId()) == H5T_STRING;

    if (is_dst_string && is_src_string) {
        if (detail::h5t_get_cset(src.getId()) == H5T_CSET_ASCII) {
            detail::h5t_set_cset(dst.getId(), H5T_CSET_ASCII);
        }
    }
}

template <>
struct string_type_checker<void> {
    inline static DataType getDataType(const DataType& element_type, const DataType& dtype) {
        if (detail::h5t_get_class(element_type.getId()) == H5T_STRING) {
            enforce_ascii_hack(element_type, dtype);
        }
        return element_type;
    }
};

template <>
struct string_type_checker<std::string> {
    inline static DataType getDataType(const DataType&, const DataType& file_datatype) {
        // The StringBuffer ensures that the data is transformed such that it
        // matches the datatype of the dataset, i.e. `file_datatype` and
        // `mem_datatype` are the same.
        return file_datatype;
    }
};

template <std::size_t FixedLen>
struct string_type_checker<char[FixedLen]> {
    inline static DataType getDataType(const DataType& element_type, const DataType& dtype) {
        DataType return_type = (dtype.isFixedLenStr()) ? AtomicType<char[FixedLen]>()
                                                       : element_type;
        enforce_ascii_hack(return_type, dtype);
        return return_type;
    }
};

template <>
struct string_type_checker<char*> {
    inline static DataType getDataType(const DataType&, const DataType& dtype) {
        if (dtype.isFixedLenStr()) {
            throw DataSetException("Can't output variable-length to fixed-length strings");
        }
        DataType return_type = AtomicType<std::string>();
        enforce_ascii_hack(return_type, dtype);
        return return_type;
    }
};

template <typename T>
template <class F>
BufferInfo<T>::BufferInfo(const DataType& dtype, F getName, Operation _op)
    : op(_op)
    , is_fixed_len_string(dtype.isFixedLenStr())
    // In case we are using Fixed-len strings we need to subtract one dimension
    , n_dimensions(details::inspector<type_no_const>::recursive_ndim -
                   ((is_fixed_len_string && is_char_array) ? 1 : 0))
    , data_type(
          string_type_checker<char_array_t>::getDataType(create_datatype<elem_type>(), dtype)) {
    // We warn. In case they are really not convertible an exception will rise on read/write
    if (dtype.getClass() != data_type.getClass()) {
        HIGHFIVE_LOG_WARN(getName() + "\": data and hdf5 dataset have different types: " +
                          data_type.string() + " -> " + dtype.string());
    } else if ((dtype.getClass() & data_type.getClass()) == DataTypeClass::Float) {
        HIGHFIVE_LOG_WARN_IF(
            (op == read) && (dtype.getSize() > data_type.getSize()),
            getName() + "\": hdf5 dataset has higher floating point precision than data on read: " +
                dtype.string() + " -> " + data_type.string());

        HIGHFIVE_LOG_WARN_IF(
            (op == write) && (dtype.getSize() < data_type.getSize()),
            getName() +
                "\": data has higher floating point precision than hdf5 dataset on write: " +
                data_type.string() + " -> " + dtype.string());
    }
}

}  // namespace details

}  // namespace HighFive
