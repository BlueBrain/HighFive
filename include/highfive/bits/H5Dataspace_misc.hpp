/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <array>
#include <initializer_list>
#include <vector>
#include <numeric>

#include <H5Spublic.h>

#include "H5Utils.hpp"
#include "H5Converter_misc.hpp"
#include "h5s_wrapper.hpp"

namespace HighFive {

namespace detail {
inline DataSpace make_data_space(hid_t hid) {
    return DataSpace::fromId(hid);
}
}  // namespace detail

inline DataSpace::DataSpace(const std::vector<size_t>& dims)
    : DataSpace(dims.begin(), dims.end()) {}

template <size_t N>
inline DataSpace::DataSpace(const std::array<size_t, N>& dims)
    : DataSpace(dims.begin(), dims.end()) {}

inline DataSpace::DataSpace(const std::initializer_list<size_t>& items)
    : DataSpace(std::vector<size_t>(items)) {}

template <typename... Args>
inline DataSpace::DataSpace(size_t dim1, Args... dims)
    : DataSpace(std::vector<size_t>{dim1, static_cast<size_t>(dims)...}) {}

template <class IT, typename>
inline DataSpace::DataSpace(const IT begin, const IT end) {
    std::vector<hsize_t> real_dims(begin, end);

    _hid = detail::h5s_create_simple(int(real_dims.size()), real_dims.data(), nullptr);
}

inline DataSpace DataSpace::Scalar() {
    return DataSpace(DataSpace::dataspace_scalar);
}

inline DataSpace DataSpace::Null() {
    return DataSpace(DataSpace::dataspace_null);
}

inline DataSpace::DataSpace(const std::vector<size_t>& dims, const std::vector<size_t>& maxdims) {
    if (dims.size() != maxdims.size()) {
        throw DataSpaceException("dims and maxdims must be the same length.");
    }

    std::vector<hsize_t> real_dims(dims.begin(), dims.end());
    std::vector<hsize_t> real_maxdims(maxdims.begin(), maxdims.end());

    // Replace unlimited flag with actual HDF one
    std::replace(real_maxdims.begin(),
                 real_maxdims.end(),
                 static_cast<hsize_t>(DataSpace::UNLIMITED),
                 H5S_UNLIMITED);

    _hid = detail::h5s_create_simple(int(dims.size()), real_dims.data(), real_maxdims.data());
}

inline DataSpace::DataSpace(DataSpace::DataspaceType space_type) {
    H5S_class_t h5_dataspace_type;
    switch (space_type) {
    case DataSpace::dataspace_scalar:
        h5_dataspace_type = H5S_SCALAR;
        break;
    case DataSpace::dataspace_null:
        h5_dataspace_type = H5S_NULL;
        break;
    default:
        throw DataSpaceException(
            "Invalid dataspace type: should be "
            "dataspace_scalar or dataspace_null");
    }

    _hid = detail::h5s_create(h5_dataspace_type);
}

inline DataSpace DataSpace::clone() const {
    DataSpace res;
    res._hid = detail::h5s_copy(_hid);
    return res;
}

inline size_t DataSpace::getNumberDimensions() const {
    return static_cast<size_t>(detail::h5s_get_simple_extent_ndims(_hid));
}

inline std::vector<size_t> DataSpace::getDimensions() const {
    std::vector<hsize_t> dims(getNumberDimensions());
    if (!dims.empty()) {
        detail::h5s_get_simple_extent_dims(_hid, dims.data(), nullptr);
    }
    return details::to_vector_size_t(std::move(dims));
}

inline size_t DataSpace::getElementCount() const {
    return static_cast<size_t>(detail::h5s_get_simple_extent_npoints(_hid));
}

inline std::vector<size_t> DataSpace::getMaxDimensions() const {
    std::vector<hsize_t> maxdims(getNumberDimensions());
    detail::h5s_get_simple_extent_dims(_hid, nullptr, maxdims.data());

    std::replace(maxdims.begin(),
                 maxdims.end(),
                 H5S_UNLIMITED,
                 static_cast<hsize_t>(DataSpace::UNLIMITED));
    return details::to_vector_size_t(maxdims);
}

template <typename T>
inline DataSpace DataSpace::From(const T& value) {
    auto dims = details::inspector<T>::getDimensions(value);
    return DataSpace(dims);
}

template <std::size_t N, std::size_t Width>
inline DataSpace DataSpace::FromCharArrayStrings(const char (&)[N][Width]) {
    return DataSpace(N);
}

namespace details {

/// dimension checks @internal
inline bool checkDimensions(const DataSpace& mem_space, size_t n_dim_requested) {
    return checkDimensions(mem_space.getDimensions(), n_dim_requested);
}

}  // namespace details
}  // namespace HighFive
