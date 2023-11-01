/*
 *  Copyright (c), 2017, Ali Can Demiralp <ali.demiralp@rwth-aachen.de>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <algorithm>
#include <functional>
#include <numeric>
#include <sstream>
#include <string>

#include <H5Apublic.h>
#include <H5Ppublic.h>

#include "../H5DataSpace.hpp"
#include "H5Converter_misc.hpp"
#include "H5ReadWrite_misc.hpp"
#include "H5Utils.hpp"

namespace HighFive {

inline std::string Attribute::getName() const {
    return details::get_name(
        [&](char* buffer, size_t length) { return H5Aget_name(_hid, length, buffer); });
}

inline size_t Attribute::getStorageSize() const {
    return static_cast<size_t>(H5Aget_storage_size(_hid));
}

inline DataType Attribute::getDataType() const {
    DataType res;
    res._hid = H5Aget_type(_hid);
    return res;
}

inline DataSpace Attribute::getSpace() const {
    DataSpace space;
    if ((space._hid = H5Aget_space(_hid)) < 0) {
        HDF5ErrMapper::ToException<AttributeException>("Unable to get DataSpace out of Attribute");
    }
    return space;
}

inline DataSpace Attribute::getMemSpace() const {
    return getSpace();
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
        details::BufferInfo<T>::read);

    if (!details::checkDimensions(mem_space, buffer_info.n_dimensions)) {
        std::ostringstream ss;
        ss << "Impossible to read DataSet of dimensions " << mem_space.getNumberDimensions()
           << " into arrays of dimensions " << buffer_info.n_dimensions;
        throw DataSpaceException(ss.str());
    }
    auto dims = mem_space.getDimensions();

    if (mem_space.getElementCount() == 0) {
        auto effective_dims = details::squeezeDimensions(dims,
                                                         details::inspector<T>::recursive_ndim);

        details::inspector<T>::prepare(array, effective_dims);
        return;
    }

    auto r = details::data_converter::get_reader<T>(dims, array, file_datatype);
    read(r.getPointer(), buffer_info.data_type);
    // re-arrange results
    r.unserialize(array);

    auto t = buffer_info.data_type;
    auto c = t.getClass();

    if (c == DataTypeClass::VarLen || t.isVariableStr()) {
#if H5_VERSION_GE(1, 12, 0)
        // This one have been created in 1.12.0
        (void) H5Treclaim(t.getId(), mem_space.getId(), H5P_DEFAULT, r.getPointer());
#else
        // This one is deprecated since 1.12.0
        (void) H5Dvlen_reclaim(t.getId(), mem_space.getId(), H5P_DEFAULT, r.getPointer());
#endif
    }
}

template <typename T>
inline void Attribute::read(T* array, const DataType& mem_datatype) const {
    static_assert(!std::is_const<T>::value,
                  "read() requires a non-const structure to read data into");

    if (H5Aread(getId(), mem_datatype.getId(), static_cast<void*>(array)) < 0) {
        HDF5ErrMapper::ToException<AttributeException>("Error during HDF5 Read: ");
    }
}

template <typename T>
inline void Attribute::read(T* array) const {
    using element_type = typename details::inspector<T>::base_type;
    const DataType& mem_datatype = create_and_check_datatype<element_type>();

    read(array, mem_datatype);
}

template <typename T>
inline void Attribute::write(const T& buffer) {
    const DataSpace& mem_space = getMemSpace();

    if (mem_space.getElementCount() == 0) {
        return;
    }

    auto file_datatype = getDataType();

    const details::BufferInfo<T> buffer_info(
        file_datatype,
        [this]() -> std::string { return this->getName(); },
        details::BufferInfo<T>::write);

    if (!details::checkDimensions(mem_space, buffer_info.n_dimensions)) {
        std::ostringstream ss;
        ss << "Impossible to write buffer of dimensions " << buffer_info.n_dimensions
           << " into dataset of dimensions " << mem_space.getNumberDimensions();
        throw DataSpaceException(ss.str());
    }
    auto w = details::data_converter::serialize<T>(buffer, file_datatype);
    write_raw(w.getPointer(), buffer_info.data_type);
}

template <typename T>
inline void Attribute::write_raw(const T* buffer, const DataType& mem_datatype) {
    if (H5Awrite(getId(), mem_datatype.getId(), buffer) < 0) {
        HDF5ErrMapper::ToException<DataSetException>("Error during HDF5 Write: ");
    }
}

template <typename T>
inline void Attribute::write_raw(const T* buffer) {
    using element_type = typename details::inspector<T>::base_type;
    const auto& mem_datatype = create_and_check_datatype<element_type>();

    write_raw(buffer, mem_datatype);
}

}  // namespace HighFive
