/*
 *  Copyright (c), 2023, 2024 Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#include <string>
#include <sstream>
#include <type_traits>

#include <catch2/catch_template_test_macros.hpp>

#include "data_generator.hpp"

namespace HighFive {
namespace testing {

template <class T, class = void>
struct DiffMessageTrait;

template <class T>
struct DiffMessageTrait<T, typename std::enable_if<std::is_floating_point<T>::value>::type> {
    static std::string diff(T a, T b) {
        std::stringstream sstream;
        sstream << std::scientific << " delta: " << a - b;
        return sstream.str();
    }
};

template <class T>
struct DiffMessageTrait<T, typename std::enable_if<!std::is_floating_point<T>::value>::type> {
    static std::string diff(T /* a */, T /* b */) {
        return "";
    }
};

template <class T>
std::string diff_message(T a, T b) {
    return DiffMessageTrait<T>::diff(a, b);
}

template <class Actual, class Expected, class Comp>
void compare_arrays(const Actual& actual,
                    const Expected& expected,
                    const std::vector<size_t>& dims,
                    Comp comp) {
    using actual_trait = testing::ContainerTraits<Actual>;
    using expected_trait = testing::ContainerTraits<Expected>;
    using base_type = typename actual_trait::base_type;

    auto n = testing::flat_size(dims);

    for (size_t i = 0; i < n; ++i) {
        auto indices = testing::unravel(i, dims);
        base_type actual_value = actual_trait::get(actual, indices);
        base_type expected_value = expected_trait::get(expected, indices);
        auto c = comp(actual_value, expected_value);
        if (!c) {
            std::stringstream sstream;
            sstream << std::scientific << "i = " << i << ": " << actual_value
                    << " != " << expected_value << diff_message(actual_value, expected_value);
            INFO(sstream.str());
        }
        REQUIRE(c);
    }
}

template <class Actual, class Expected>
void compare_arrays(const Actual& actual,
                    const Expected& expected,
                    const std::vector<size_t>& dims) {
    using base_type = typename testing::ContainerTraits<Actual>::base_type;
    compare_arrays(expected, actual, dims, [](base_type a, base_type b) { return a == b; });
}

}  // namespace testing
}  // namespace HighFive
