/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#include <complex>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <typeinfo>
#include <vector>

#include <stdio.h>

#include <highfive/H5Easy.hpp>

#ifdef HIGHFIVE_XTENSOR
#include <xtensor/xrandom.hpp>
#endif

#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(H5Easy_scalar)
{
    HighFive::File file("test.h5", HighFive::File::Overwrite);

    double a = 1.2345;
    int b = 12345;
    std::string c = "12345";

    HighFive::dump(file, "/path/to/a", a);
    HighFive::dump(file, "/path/to/b", b);
    HighFive::dump(file, "/path/to/c", c);

    double a_r = HighFive::load<double>(file, "/path/to/a");
    int b_r = HighFive::load<int>(file, "/path/to/b");
    std::string c_r = HighFive::load<std::string>(file, "/path/to/c");

    BOOST_CHECK_EQUAL(a == a_r, true);
    BOOST_CHECK_EQUAL(b == b_r, true);
    BOOST_CHECK_EQUAL(c == c_r, true);
}

BOOST_AUTO_TEST_CASE(H5Easy_vector)
{
    HighFive::File file("test.h5", HighFive::File::Overwrite);

    std::vector<size_t> a = {1,2,3,4,5};

    HighFive::dump(file, "/path/to/a", a);

    std::vector<size_t> a_r = HighFive::load<std::vector<size_t>>(file, "/path/to/a");

    BOOST_CHECK_EQUAL(a == a_r, true);
}

#ifdef HIGHFIVE_XTENSOR
BOOST_AUTO_TEST_CASE(H5Easy_extend1d)
{
    HighFive::File file("test.h5", HighFive::File::Overwrite);

    for (size_t i = 0; i < 10; ++i) {
        HighFive::dump(file, "/path/to/A", i, {i});
    }

    xt::xarray<size_t> A = xt::arange<size_t>(10);

    xt::xarray<size_t> A_r = HighFive::load<xt::xarray<size_t>>(file, "/path/to/A");

    size_t Amax = HighFive::load<size_t>(file, "/path/to/A", {9});

    BOOST_CHECK_EQUAL(xt::allclose(A, A_r), true);
    BOOST_CHECK_EQUAL(Amax == 9, true);
}
#endif

#ifdef HIGHFIVE_XTENSOR
BOOST_AUTO_TEST_CASE(H5Easy_extend2d)
{
    HighFive::File file("test.h5", HighFive::File::Overwrite);

    for (size_t i = 0; i < 10; ++i) {
        for (size_t j = 0; j < 5; ++j) {
            HighFive::dump(file, "/path/to/A", i * 5 + j, {i, j});
        }
    }

    xt::xarray<size_t> A = xt::arange<size_t>(10 * 5);

    A.reshape({10, 5});

    xt::xarray<size_t> A_r = HighFive::load<xt::xarray<size_t>>(file, "/path/to/A");

    size_t Amax = HighFive::load<size_t>(file, "/path/to/A", {9, 4});

    BOOST_CHECK_EQUAL(xt::allclose(A, A_r), true);
    BOOST_CHECK_EQUAL(Amax == 49, true);
}
#endif

#ifdef HIGHFIVE_XTENSOR
BOOST_AUTO_TEST_CASE(H5Easy_xtensor)
{
    HighFive::File file("test.h5", HighFive::File::Overwrite);

    xt::xtensor<double, 2> A = 100. * xt::random::randn<double>({20, 5});
    xt::xtensor<int, 2> B = A;

    HighFive::dump(file, "/path/to/A", A);
    HighFive::dump(file, "/path/to/B", B);

    xt::xtensor<double,2> A_r = HighFive::load<xt::xtensor<double,2>>(file, "/path/to/A");
    xt::xtensor<int, 2> B_r = HighFive::load<xt::xtensor<int, 2>>(file, "/path/to/B");

    BOOST_CHECK_EQUAL(xt::allclose(A, A_r), true);
    BOOST_CHECK_EQUAL(xt::all(xt::equal(B, B_r)), true);
}
#endif

#ifdef HIGHFIVE_XTENSOR
BOOST_AUTO_TEST_CASE(H5Easy_xarray)
{
    HighFive::File file("test.h5", HighFive::File::Overwrite);

    xt::xarray<double> A = 100. * xt::random::randn<double>({20, 5});
    xt::xarray<int> B = A;

    HighFive::dump(file, "/path/to/A", A);
    HighFive::dump(file, "/path/to/B", B);

    xt::xarray<double> A_r = HighFive::load<xt::xarray<double>>(file, "/path/to/A");
    xt::xarray<int> B_r = HighFive::load<xt::xarray<int>>(file, "/path/to/B");

    BOOST_CHECK_EQUAL(xt::allclose(A, A_r), true);
    BOOST_CHECK_EQUAL(xt::all(xt::equal(B, B_r)), true);
}
#endif

#ifdef HIGHFIVE_EIGEN
BOOST_AUTO_TEST_CASE(H5Easy_Eigen_MatrixX)
{
    HighFive::File file("test.h5", HighFive::File::Overwrite);

    Eigen::MatrixXd A = 100. * Eigen::MatrixXd::Random(20, 5);
    Eigen::MatrixXi B = A.cast<int>();

    HighFive::dump(file, "/path/to/A", A);
    HighFive::dump(file, "/path/to/B", B);

    Eigen::MatrixXd A_r = HighFive::load<Eigen::MatrixXd>(file, "/path/to/A");
    Eigen::MatrixXi B_r = HighFive::load<Eigen::MatrixXi>(file, "/path/to/B");

    BOOST_CHECK_EQUAL(A.isApprox(A_r), true);
    BOOST_CHECK_EQUAL(B.isApprox(B_r), true);
}
#endif
