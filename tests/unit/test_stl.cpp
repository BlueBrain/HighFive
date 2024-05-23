/*
 *  Copyright (c), 2017-2024, Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#include <catch2/catch_template_test_macros.hpp>

#include <highfive/highfive.hpp>


#include "compary_arrays.hpp"
#include "create_traits.hpp"
#include "data_generator.hpp"

using namespace HighFive;

TEST_CASE("std::array undersized", "[stl]") {
    auto file = File("rw_std_array_undersized.h5", File::Truncate);
    auto x = std::array<double, 3>{1.0, 2.0, 3.0};
    auto dset = file.createDataSet("x", x);

    REQUIRE_THROWS(dset.read<std::array<double, 2>>());

    auto xx = std::array<double, 2>();
    REQUIRE_THROWS(dset.read(xx));
}

TEST_CASE("T[n][m]") {
    using reference_container = std::vector<std::vector<double>>;
    auto file = File("rw_carray.h5", File::Truncate);

    constexpr size_t n = 3;
    constexpr size_t m = 5;

    double x[n][m];

    SECTION("write") {
        testing::initialize(x, {n, m});

        auto dset = file.createDataSet("x", x);
        auto actual = dset.read<reference_container>();

        testing::compare_arrays(x, actual, {n, m});
    }

    SECTION("read") {
        auto expected = testing::DataGenerator<reference_container>::create({n, m});

        auto dset = file.createDataSet("x", expected);
        dset.read(x);

        testing::compare_arrays(expected, x, {n, m});
    }
}
