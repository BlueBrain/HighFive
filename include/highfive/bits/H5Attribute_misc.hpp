/*
 *  Copyright (c), 2017, Ali Can Demiralp <ali.demiralp@rwth-aachen.de>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <H5Ipublic.h>
#include <algorithm>
#include <functional>
#include <numeric>
#include <sstream>
#include <string>

#include <H5Ppublic.h>

#include "../H5DataSpace.hpp"
#include "H5Converter_misc.hpp"
#include "H5Inspector_misc.hpp"
#include "H5ReadWrite_misc.hpp"
#include "H5Utils.hpp"
#include "h5a_wrapper.hpp"
#include "h5d_wrapper.hpp"
#include "squeeze.hpp"
#include "assert_compatible_spaces.hpp"

namespace HighFive {

inline std::string Attribute::getName() const {
    return details::get_name(
        [&](char* buffer, size_t length) { return detail::h5a_get_name(_hid, length, buffer); });
}

inline size_t Attribute::getStorageSize() const {
    if (!this->isValid()) {
        throw AttributeException("Invalid call to `DataSet::getFile` for invalid object");
    }

    return static_cast<size_t>(detail::h5a_get_storage_size(_hid));
}

inline DataType Attribute::getDataType() const {
    DataType res;
    res._hid = detail::h5a_get_type(_hid);
    return res;
}

inline DataSpace Attribute::getSpace() const {
    DataSpace space;
    space._hid = detail::h5a_get_space(_hid);
    return space;
}

inline DataSpace Attribute::getMemSpace() const {
    return _mem_space.getId() == H5I_INVALID_HID ? getSpace() : _mem_space;
}

template <typename T>
inline T Attribute::read() const {
    T array;
    read(array);
    return array;
}

template <typename T>
inline void Attribute::read(T& array) const {
    const DataSpace& mem_space = getMemSpace();
    auto file_datatype = getDataType();
    const details::BufferInfo<T> buffer_info(
        file_datatype,
        [this]() -> std::string { return this->getName(); },
        details::BufferInfo<T>::Operation::read);

    if (!details::checkDimensions(mem_space, buffer_info.getMinRank(), buffer_info.getMaxRank())) {
        std::ostringstream ss;
        ss << "Impossible to read attribute of dimensions " << mem_space.getNumberDimensions()
           << " into arrays of dimensions: " << buffer_info.getMinRank() << "(min) to "
           << buffer_info.getMaxRank() << "(max)";
        throw DataSpaceException(ss.str());
    }
    auto dims = mem_space.getDimensions();

    if (mem_space.getElementCount() == 0) {
        details::inspector<T>::prepare(array, dims);
        return;
    }

    auto r = details::data_converter::get_reader<T>(dims, array, file_datatype);
    read_raw(r.getPointer(), buffer_info.data_type);
    // re-arrange results
    r.unserialize(array);

    auto t = buffer_info.data_type;
    auto c = t.getClass();

    if (c == DataTypeClass::VarLen || t.isVariableStr()) {
#if H5_VERSION_GE(1, 12, 0)
        // This one have been created in 1.12.0
        (void) detail::h5t_reclaim(t.getId(), mem_space.getId(), H5P_DEFAULT, r.getPointer());
#else
        // This one is deprecated since 1.12.0
        (void) detail::h5d_vlen_reclaim(t.getId(), mem_space.getId(), H5P_DEFAULT, r.getPointer());
#endif
    }
}

template <typename T>
inline void Attribute::read_raw(T* array, const DataType& mem_datatype) const {
    static_assert(!std::is_const<T>::value,
                  "read() requires a non-const structure to read data into");

    detail::h5a_read(getId(), mem_datatype.getId(), static_cast<void*>(array));
}

template <typename T>
inline void Attribute::read_raw(T* array) const {
    using element_type = typename details::inspector<T>::base_type;
    const DataType& mem_datatype = create_and_check_datatype<element_type>();

    read_raw(array, mem_datatype);
}

template <typename T>
inline void Attribute::write(const T& buffer) {
    const DataSpace& mem_space = getMemSpace();
    auto dims = mem_space.getDimensions();

    if (mem_space.getElementCount() == 0) {
        return;
    }

    auto file_datatype = getDataType();

    const details::BufferInfo<T> buffer_info(
        file_datatype,
        [this]() -> std::string { return this->getName(); },
        details::BufferInfo<T>::Operation::write);

    if (!details::checkDimensions(mem_space, buffer_info.getMinRank(), buffer_info.getMaxRank())) {
        std::ostringstream ss;
        ss << "Impossible to write attribute of dimensions " << mem_space.getNumberDimensions()
           << " into arrays of dimensions: " << buffer_info.getMinRank() << "(min) to "
           << buffer_info.getMaxRank() << "(max)";
        throw DataSpaceException(ss.str());
    }
    auto w = details::data_converter::serialize<T>(buffer, dims, file_datatype);
    write_raw(w.getPointer(), buffer_info.data_type);
}

template <typename T>
inline void Attribute::write_raw(const T* buffer, const DataType& mem_datatype) {
    detail::h5a_write(getId(), mem_datatype.getId(), buffer);
}

template <typename T>
inline void Attribute::write_raw(const T* buffer) {
    using element_type = typename details::inspector<T>::base_type;
    const auto& mem_datatype = create_and_check_datatype<element_type>();

    write_raw(buffer, mem_datatype);
}

inline Attribute Attribute::squeezeMemSpace(const std::vector<size_t>& axes) const {
    auto mem_dims = this->getMemSpace().getDimensions();
    auto squeezed_dims = detail::squeeze(mem_dims, axes);

    auto attr = *this;
    attr._mem_space = DataSpace(squeezed_dims);
    return attr;
}

inline Attribute Attribute::reshapeMemSpace(const std::vector<size_t>& new_dims) const {
    detail::assert_compatible_spaces(this->getMemSpace(), new_dims);

    auto attr = *this;
    attr._mem_space = DataSpace(new_dims);
    return attr;
}

}  // namespace HighFive
