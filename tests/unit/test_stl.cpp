/*
 *  Copyright (c), 2017-2024, Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#include <highfive/highfive.hpp>

using namespace HighFive;

TEST_CASE("std::array undersized", "[stl]") {
    auto file = File("rw_std_array_undersized.h5");
    auto x = std::array<double, 3>{1.0, 2.0, 3.0};
    auto dset = file.createDataSet("x", x);

    REQUIRE_THROWS(dset.read<std::array<double, 2>>());

    auto xx = std::array<double, 2>();
    REQUIRE_THROWS(dset.read(xx));
}
