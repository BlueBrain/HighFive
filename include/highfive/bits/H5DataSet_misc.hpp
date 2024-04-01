/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
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

#include <H5Ppublic.h>

#include "h5d_wrapper.hpp"
#include "H5Utils.hpp"

namespace HighFive {

inline uint64_t DataSet::getStorageSize() const {
    if (!this->isValid()) {
        throw DataSetException("Invalid call to `DataSet::getStorageSize` for invalid object");
    }

    return detail::h5d_get_storage_size(_hid);
}

inline DataType DataSet::getDataType() const {
    return DataType(detail::h5d_get_type(_hid));
}

inline DataSpace DataSet::getSpace() const {
    DataSpace space;
    space._hid = detail::h5d_get_space(_hid);
    return space;
}

inline DataSpace DataSet::getMemSpace() const {
    return getSpace();
}

inline uint64_t DataSet::getOffset() const {
    return static_cast<uint64_t>(detail::h5d_get_offset(_hid));
}

inline void DataSet::resize(const std::vector<size_t>& dims) {
    const size_t numDimensions = getSpace().getDimensions().size();
    if (dims.size() != numDimensions) {
        HDF5ErrMapper::ToException<DataSetException>("Invalid dataspace dimensions, got " +
                                                     std::to_string(dims.size()) + " expected " +
                                                     std::to_string(numDimensions));
    }

    std::vector<hsize_t> real_dims(dims.begin(), dims.end());
    detail::h5d_set_extent(getId(), real_dims.data());
}

}  // namespace HighFive
