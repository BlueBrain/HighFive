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

namespace HighFive {
namespace detail {

/// \brief Squeeze `axes` from `dims`.
///
/// An axis can only be squeezed if it's dimension is `1`. The elements of
/// `axes` must be in the range `0, ..., dims.size()` (exclusive) and don't
/// have to be sorted.
///
/// Example:
///   squeeze({1, 3, 2, 1}, {0, 3}) == {3, 2}
inline std::vector<size_t> squeeze(const std::vector<size_t>& dims,
                                   const std::vector<size_t>& axes) {
    auto n_dims = dims.size();
    auto mask = std::vector<bool>(n_dims, false);
    for (size_t i = 0; i < axes.size(); ++i) {
        if (axes[i] >= n_dims) {
            throw Exception("Out of range: axes[" + std::to_string(i) +
                            "] == " + std::to_string(axes[i]) + " >= " + std::to_string(n_dims));
        }

        mask[axes[i]] = true;
    }

    auto squeezed_dims = std::vector<size_t>{};
    for (size_t i = 0; i < n_dims; ++i) {
        if (!mask[i]) {
            squeezed_dims.push_back(dims[i]);
        } else {
            if (dims[i] != 1) {
                throw Exception("Squeezing non-unity axis: axes[" + std::to_string(i) +
                                "] = " + std::to_string(axes[i]));
            }
        }
    }

    return squeezed_dims;
}

}  // namespace detail
}  // namespace HighFive
