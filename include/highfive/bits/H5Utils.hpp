/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

// internal utilities functions
#include <algorithm>
#include <array>
#include <cstddef>  // __GLIBCXX__
#include <exception>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>

#include <H5public.h>

#include "../H5Exception.hpp"

namespace HighFive {

// If ever used, recognize dimensions of FixedLenStringArray
template <std::size_t N>
class FixedLenStringArray;

namespace details {
// converter function for hsize_t -> size_t when hsize_t != size_t
template <typename Size>
inline std::vector<std::size_t> to_vector_size_t(const std::vector<Size>& vec) {
    static_assert(std::is_same<Size, std::size_t>::value == false,
                  " hsize_t != size_t mandatory here");
    std::vector<size_t> res(vec.size());
    std::transform(vec.cbegin(), vec.cend(), res.begin(), [](Size e) {
        return static_cast<size_t>(e);
    });
    return res;
}

// converter function for hsize_t -> size_t when size_t == hsize_t
inline std::vector<std::size_t> to_vector_size_t(const std::vector<std::size_t>& vec) {
    return vec;
}

// converter function for size_t -> hsize_t when size_t != hsize_t
template <typename Size>
inline std::vector<hsize_t> to_vector_hsize_t(const std::vector<Size>& vec) {
    static_assert(std::is_same<Size, hsize_t>::value == false, " size_t != hsize_t mandatory here");
    std::vector<hsize_t> res(vec.size());
    std::transform(vec.cbegin(), vec.cend(), res.begin(), [](Size e) {
        return static_cast<hsize_t>(e);
    });
    return res;
}

// converter function for size_t -> hsize_t when hsize_t == size_t
inline std::vector<hsize_t> to_vector_hsize_t(const std::vector<hsize_t>& vec) {
    return vec;
}

inline hsize_t compute_total_size(const std::vector<hsize_t>& dims) {
    return std::accumulate(dims.begin(), dims.end(), size_t{1u}, std::multiplies<std::size_t>());
}

inline size_t compute_total_size(const std::vector<std::size_t>& dims) {
    return std::accumulate(dims.begin(), dims.end(), size_t{1u}, std::multiplies<std::size_t>());
}

// read name from a H5 object using the specified function
template <typename T>
inline std::string get_name(T fct) {
    const size_t maxLength = 255;
    char buffer[maxLength + 1];
    ssize_t retcode = fct(buffer, static_cast<hsize_t>(maxLength) + 1);
    if (retcode < 0) {
        HDF5ErrMapper::ToException<GroupException>("Error accessing object name");
    }
    const size_t length = static_cast<std::size_t>(retcode);
    if (length <= maxLength) {
        return std::string(buffer, length);
    }
    std::vector<char> bigBuffer(length + 1, 0);
    fct(bigBuffer.data(), length + 1);
    return std::string(bigBuffer.data(), length);
}

}  // namespace details
}  // namespace HighFive
