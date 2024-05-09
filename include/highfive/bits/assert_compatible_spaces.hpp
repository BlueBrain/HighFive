/*
 *  Copyright (c), 2024, BlueBrain Project, EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <vector>
#include "../H5Exception.hpp"
#include "../H5DataSpace.hpp"

namespace HighFive {
namespace detail {

inline void assert_compatible_spaces(const DataSpace& old, const std::vector<size_t>& dims) {
    auto n_elements_old = old.getElementCount();
    auto n_elements_new = dims.size() == 0 ? 1 : compute_total_size(dims);

    if (n_elements_old != n_elements_new) {
        throw Exception("Invalid parameter `new_dims` number of elements differ: " +
                        std::to_string(n_elements_old) + " (old) vs. " +
                        std::to_string(n_elements_new) + " (new)");
    }
}
}  // namespace detail
}  // namespace HighFive
