/*
 *  Copyright (c), 2024, Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#if HIGHFIVE_TEST_XTENSOR
#include <string>
#include <sstream>

#include <catch2/catch_template_test_macros.hpp>

#include <highfive/highfive.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>
#include <xtensor/xio.hpp>
#include <highfive/xtensor.hpp>

#include "data_generator.hpp"

using namespace HighFive;

template <size_t N>
std::array<size_t, N> asStaticShape(const std::vector<size_t>& dims) {
    assert(dims.size() == N);

    std::array<size_t, N> shape;
    std::copy(dims.cbegin(), dims.cend(), shape.begin());

    return shape;
}

TEST_CASE("xt::xarray reshape", "[xtensor]") {
    const std::string file_name("rw_dataset_xarray.h5");

    File file(file_name, File::Truncate);

    std::vector<size_t> shape{3, 2, 4};
    std::vector<size_t> compatible_shape{1, 3, 2, 4};
    std::vector<size_t> incompatible_shape{5, 2, 4};

    xt::xarray<double> a = testing::DataGenerator<xt::xtensor<double, 3>>::create(shape);
    xt::xarray<double> b(compatible_shape);
    xt::xarray<double> c(incompatible_shape);

    auto dset = file.createDataSet("baz", a);

    SECTION("xarray_adaptor") {
        // Changes the shape.
        auto b_adapt = xt::adapt(b.data(), b.size(), xt::no_ownership(), b.shape());
        dset.read(b_adapt);
        REQUIRE(b_adapt.shape() == shape);

        // But can't change the number of elements.
        auto c_adapt = xt::adapt(c.data(), c.size(), xt::no_ownership(), c.shape());
        REQUIRE_THROWS(dset.read(c_adapt));
    }

    SECTION("xtensor_adaptor") {
        auto b_shape = asStaticShape<4>(compatible_shape);
        auto c_shape = asStaticShape<3>(incompatible_shape);

        // Doesn't change the shape:
        auto b_adapt = xt::adapt(b.data(), b.size(), xt::no_ownership(), b_shape);
        REQUIRE_THROWS(dset.read(b_adapt));

        // and can't change the number of elements:
        auto c_adapt = xt::adapt(c.data(), c.size(), xt::no_ownership(), c_shape);
        REQUIRE_THROWS(dset.read(c_adapt));
    }
}

TEST_CASE("xt::xview example", "[xtensor]") {
    File file("rw_dataset_xview.h5", File::Truncate);

    std::vector<size_t> shape{13, 5, 7};
    xt::xarray<double> a = testing::DataGenerator<xt::xtensor<double, 3>>::create(shape);
    auto c = xt::view(a, xt::range(3, 31, 4), xt::all(), xt::drop(0, 3, 4, 5));

    auto dset = file.createDataSet("c", c);
    auto d = dset.read<xt::xarray<double>>();
    auto e = dset.read<xt::xarray<double, xt::layout_type::column_major>>();

    REQUIRE(d == c);
    REQUIRE(e == c);
}

template <class XTensor>
void check_xtensor_scalar(File& file) {
    XTensor a;
    a = 42.0;
    REQUIRE(a.shape() == std::vector<size_t>{});

    SECTION("read") {
        auto dset = file.createDataSet("a", a);
        REQUIRE(dset.template read<double>() == a(0));
    }

    SECTION("write") {
        double b = -42.0;
        auto dset = file.createDataSet("b", b);
        REQUIRE(dset.template read<xt::xarray<double>>()(0) == b);
    }
}

TEST_CASE("xt::xarray scalar", "[xtensor]") {
    File file("rw_dataset_xarray_scalar.h5", File::Truncate);
    check_xtensor_scalar<xt::xarray<double>>(file);
}

TEST_CASE("xt::xtensor scalar", "[xtensor]") {
    File file("rw_dataset_xtensor_scalar.h5", File::Truncate);
    check_xtensor_scalar<xt::xarray<double>>(file);
}

template <class XTensor>
void check_xtensor_empty(File& file, const XTensor& a, const std::vector<size_t>& expected_dims) {
    auto dset = file.createDataSet("a", a);
    auto b = dset.template read<XTensor>();
    REQUIRE(b.size() == 0);
    REQUIRE(b == a);

    auto c = std::vector<XTensor>{};
    auto c_shape = details::inspector<decltype(c)>::getDimensions(c);
    REQUIRE(c_shape == expected_dims);
}

TEST_CASE("xt::xtensor empty", "[xtensor]") {
    File file("rw_dataset_xtensor_empty.h5", File::Truncate);
    xt::xtensor<double, 3> a({0, 1, 1});
    check_xtensor_empty(file, a, {0, 1, 1, 1});
}

TEST_CASE("xt::xarray empty", "[xtensor]") {
    File file("rw_dataset_xarray_empty.h5", File::Truncate);
    xt::xarray<double> a(std::vector<size_t>{1, 0, 1});
    check_xtensor_empty(file, a, {0});
}

#endif
