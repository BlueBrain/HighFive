/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *  Copyright (c), 2017-2024, BlueBrain Project, EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <vector>

namespace HighFive {
namespace detail {

template <class To, class From, class It = From const*>
inline std::vector<To> convertSizeVector(const It& begin, const It& end) {
    std::vector<To> to(static_cast<size_t>(end - begin));
    std::copy(begin, end, to.begin());

    return to;
}

template <class To, class From>
inline std::vector<To> convertSizeVector(const std::vector<From>& from) {
    return convertSizeVector<To, From>(from.cbegin(), from.cend());
}

}  // namespace detail
}  // namespace HighFive
