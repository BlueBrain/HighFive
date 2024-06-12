/*
 *  Copyright (c), 2024, Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#if HIGHFIVE_TEST_OPENCV

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <highfive/highfive.hpp>
#include <highfive/experimental/opencv.hpp>
#include "tests_high_five.hpp"
#include "create_traits.hpp"

using namespace HighFive;
using Catch::Matchers::Equals;

TEST_CASE("OpenCV") {
    auto file = File("rw_opencv.h5", File::Truncate);

    auto a = cv::Mat_<double>(3, 5);
    auto dset = file.createDataSet("a", a);
    auto b = dset.read<cv::Mat_<double>>();
    REQUIRE(a(0, 0) == b(0, 0));

    auto va = std::vector<cv::Mat_<double>>(7, cv::Mat_<double>(3, 5));
    auto vdset = file.createDataSet("va", va);
    auto vb = vdset.read<std::vector<cv::Mat_<double>>>();
    REQUIRE(vb.size() == va.size());
    REQUIRE(vb[0](0, 0) == va[0](0, 0));
}

TEST_CASE("OpenCV subarrays") {
    auto file = File("rw_opencv_subarray.h5", File::Truncate);

    auto a = cv::Mat_<double>(3, 13);

    SECTION("write") {
        auto sa = cv::Mat_<double>(a.colRange(1, 4));
        REQUIRE_THROWS(file.createDataSet("a", sa));
    }

    SECTION("read") {
        auto b = cv::Mat_<double>(3, 17);
        auto sb = cv::Mat_<double>(a.colRange(0, 13));
        auto dset = file.createDataSet("a", a);

        // Creates a new `Mat_` in `sb`.
        dset.read(sb);
    }
}

#endif
