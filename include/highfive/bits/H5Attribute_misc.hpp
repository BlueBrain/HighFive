/*
 *  Copyright (c), 2017, Ali Can Demiralp <ali.demiralp@rwth-aachen.de>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5ATTRIBUTE_MISC_HPP
#define H5ATTRIBUTE_MISC_HPP

#include <algorithm>
#include <functional>
#include <numeric>
#include <sstream>
#include <string>

#ifdef H5_USE_BOOST
#include <boost/multi_array.hpp>
#endif

#include <H5Apublic.h>
#include <H5Ppublic.h>

#include "H5Converter_misc.hpp"
#include "H5Utils.hpp"

namespace HighFive {

inline std::string Attribute::getName() const {
    return details::get_name([&](char *buffer, hsize_t length) {
        return H5Aget_name(_hid, length, buffer);
    });
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
        HDF5ErrMapper::ToException<AttributeException>(
            "Unable to get DataSpace out of Attribute");
    }
    return space;
}

inline DataSpace Attribute::getMemSpace() const { return getSpace(); }

template <typename T>
inline T Attribute::read() const {
    // Apply pre read conversions
    DataSpace mem_space = getMemSpace();
    auto converter = make_transform_read<T>(mem_space);

    read(converter.get_pointer(), converter._h5_type);

    // re-arrange results
    return converter.transform_read();
}

template <typename T>
inline void Attribute::read(T* array, const DataType& dtype) const {
    static_assert(!std::is_const<T>::value,
                  "read() requires a non-const structure to read data into");
    const auto& mem_datatype = dtype.empty() ? create_and_check_datatype<T>() : dtype;

    if (H5Aread(getId(), mem_datatype.getId(),
                static_cast<void*>(array)) < 0) {
        HDF5ErrMapper::ToException<AttributeException>(
            "Error during HDF5 Read: ");
    }
}

template <typename T>
inline void Attribute::write(const T& buffer) {
    DataSpace mem_space = getMemSpace();
    auto converter = make_transform_write(mem_space, buffer);

    if (!details::checkDimensions(mem_space, converter.n_dims)) {
        std::ostringstream ss;
        ss << "Impossible to write buffer of dimensions " << converter.n_dims
           << " into attribute of dimensions "
           << mem_space.getNumberDimensions();
        throw DataSpaceException(ss.str());
    }

    write_raw(converter.get_pointer(), converter._h5_type);
}

template <typename T>
inline void Attribute::write_raw(const T* buffer, const DataType& dtype) {
    const DataType mem_datatype = dtype.empty() ? create_and_check_datatype<T>() : dtype;

    if (H5Awrite(getId(), mem_datatype.getId(), buffer) < 0) {
        HDF5ErrMapper::ToException<DataSetException>(
            "Error during HDF5 Write: ");
    }
}

}  // namespace HighFive

#endif // H5ATTRIBUTE_MISC_HPP
