#pragma once

#include <cstddef>
#include <numeric>
#include <functional>
#include <vector>

namespace HighFive {

inline size_t compute_total_size(const std::vector<size_t>& dims) {
    return std::accumulate(dims.begin(), dims.end(), size_t{1u}, std::multiplies<size_t>());
}

template <typename T>
using unqualified_t = typename std::remove_const<typename std::remove_reference<T>::type>::type;


namespace details {

template <typename T>
struct type_helper;

template <typename T>
struct inspector;

}  // namespace details
}  // namespace HighFive
