#pragma once

#include <cstddef>
#include <numeric>
#include <functional>
#include <vector>

namespace HighFive {

inline size_t compute_total_size(const std::vector<size_t>& dims) {
    return std::accumulate(dims.begin(), dims.end(), size_t{1u}, std::multiplies<size_t>());
}

}  // namespace HighFive
