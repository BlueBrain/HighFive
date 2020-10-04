/*
 *  Copyright (c) 2020 Blue Brain Project
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include "H5Utils.hpp"

namespace HighFive {

namespace details {

// details implementation
template <typename SrcStrT>
struct string_type_checker {
    static DataType getDataType(const DataType&, bool);
};

template <>
struct string_type_checker<void> {
inline static DataType getDataType(const DataType& element_type, bool) {
    return element_type;
}};

template <std::size_t FixedLen>
struct string_type_checker<char[FixedLen]> {
inline static DataType getDataType(const DataType& element_type, bool ds_fixed_str) {
    return ds_fixed_str ? AtomicType<char[FixedLen]>() : element_type;
}};

template <>
struct string_type_checker<char*> {
inline static DataType getDataType(const DataType&, bool ds_fixed_str) {
    if (ds_fixed_str)
        throw DataSetException("Can't output variable-length to fixed-length strings");
    return AtomicType<std::string>();
}};

}  // namespace details

}  // namespace HighFive
