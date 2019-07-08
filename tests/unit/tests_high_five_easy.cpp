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

#ifdef H5_USE_XTENSOR
#include <xtensor/xrandom.hpp>
#endif


#define BOOST_TEST_MAIN H5EasyTests


#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(H5Easy_scalar)
{
    H5Easy::File file("test.h5", H5Easy::File::Overwrite);

    double a = 1.2345;
    int b = 12345;
    std::string c = "12345";

    H5Easy::dump(file, "/path/to/a", a);
    H5Easy::dump(file, "/path/to/b", b);
    H5Easy::dump(file, "/path/to/c", c);

    double a_r = H5Easy::load<double>(file, "/path/to/a");
    int b_r = H5Easy::load<int>(file, "/path/to/b");
    std::string c_r = H5Easy::load<std::string>(file, "/path/to/c");

    BOOST_CHECK_EQUAL(a == a_r, true);
    BOOST_CHECK_EQUAL(b == b_r, true);
    BOOST_CHECK_EQUAL(c == c_r, true);
}

BOOST_AUTO_TEST_CASE(H5Easy_vector)
{
    H5Easy::File file("test.h5", H5Easy::File::Overwrite);

    std::vector<size_t> a = {1,2,3,4,5};

    H5Easy::dump(file, "/path/to/a", a);

    std::vector<size_t> a_r = H5Easy::load<std::vector<size_t>>(file, "/path/to/a");

    BOOST_CHECK_EQUAL(a == a_r, true);
}

#ifdef H5_USE_XTENSOR
BOOST_AUTO_TEST_CASE(H5Easy_extend1d)
{
    H5Easy::File file("test.h5", H5Easy::File::Overwrite);

    for (size_t i = 0; i < 10; ++i) {
        H5Easy::dump(file, "/path/to/A", i, {i});
    }

    xt::xarray<size_t> A = xt::arange<size_t>(10);

    xt::xarray<size_t> A_r = H5Easy::load<xt::xarray<size_t>>(file, "/path/to/A");

    size_t Amax = H5Easy::load<size_t>(file, "/path/to/A", {9});

    BOOST_CHECK_EQUAL(xt::allclose(A, A_r), true);
    BOOST_CHECK_EQUAL(Amax == 9, true);
}

BOOST_AUTO_TEST_CASE(H5Easy_extend2d)
{
    H5Easy::File file("test.h5", H5Easy::File::Overwrite);

    for (size_t i = 0; i < 10; ++i) {
        for (size_t j = 0; j < 5; ++j) {
            H5Easy::dump(file, "/path/to/A", i * 5 + j, {i, j});
        }
    }

    xt::xarray<size_t> A = xt::arange<size_t>(10 * 5);

    A.reshape({10, 5});

    xt::xarray<size_t> A_r = H5Easy::load<xt::xarray<size_t>>(file, "/path/to/A");

    size_t Amax = H5Easy::load<size_t>(file, "/path/to/A", {9, 4});

    BOOST_CHECK_EQUAL(xt::allclose(A, A_r), true);
    BOOST_CHECK_EQUAL(Amax == 49, true);
}

BOOST_AUTO_TEST_CASE(H5Easy_xtensor)
{
    H5Easy::File file("test.h5", H5Easy::File::Overwrite);

    xt::xtensor<double, 2> A = 100. * xt::random::randn<double>({20, 5});
    xt::xtensor<int, 2> B = A;

    H5Easy::dump(file, "/path/to/A", A);
    H5Easy::dump(file, "/path/to/B", B);

    xt::xtensor<double,2> A_r = H5Easy::load<xt::xtensor<double,2>>(file, "/path/to/A");
    xt::xtensor<int, 2> B_r = H5Easy::load<xt::xtensor<int, 2>>(file, "/path/to/B");

    BOOST_CHECK_EQUAL(xt::allclose(A, A_r), true);
    BOOST_CHECK_EQUAL(xt::all(xt::equal(B, B_r)), true);
}

BOOST_AUTO_TEST_CASE(H5Easy_xarray)
{
    H5Easy::File file("test.h5", H5Easy::File::Overwrite);

    xt::xarray<double> A = 100. * xt::random::randn<double>({20, 5});
    xt::xarray<int> B = A;

    H5Easy::dump(file, "/path/to/A", A);
    H5Easy::dump(file, "/path/to/B", B);

    xt::xarray<double> A_r = H5Easy::load<xt::xarray<double>>(file, "/path/to/A");
    xt::xarray<int> B_r = H5Easy::load<xt::xarray<int>>(file, "/path/to/B");

    BOOST_CHECK_EQUAL(xt::allclose(A, A_r), true);
    BOOST_CHECK_EQUAL(xt::all(xt::equal(B, B_r)), true);
}
#endif

#ifdef H5_USE_EIGEN
BOOST_AUTO_TEST_CASE(H5Easy_Eigen_MatrixX)
{
    H5Easy::File file("test.h5", H5Easy::File::Overwrite);

    Eigen::MatrixXd A = 100. * Eigen::MatrixXd::Random(20, 5);
    Eigen::MatrixXi B = A.cast<int>();

    H5Easy::dump(file, "/path/to/A", A);
    H5Easy::dump(file, "/path/to/B", B);

    Eigen::MatrixXd A_r = H5Easy::load<Eigen::MatrixXd>(file, "/path/to/A");
    Eigen::MatrixXi B_r = H5Easy::load<Eigen::MatrixXi>(file, "/path/to/B");

    BOOST_CHECK_EQUAL(A.isApprox(A_r), true);
    BOOST_CHECK_EQUAL(B.isApprox(B_r), true);
}
#endif

#ifdef H5_USE_EIGEN
BOOST_AUTO_TEST_CASE(H5Easy_Eigen_VectorX)
{
    H5Easy::File file("test.h5", H5Easy::File::Overwrite);

    Eigen::VectorXd A = 100. * Eigen::VectorXd::Random(20);
    Eigen::VectorXi B = A.cast<int>();

    H5Easy::dump(file, "/path/to/A", A);
    H5Easy::dump(file, "/path/to/B", B);

    Eigen::VectorXd A_r = H5Easy::load<Eigen::VectorXd>(file, "/path/to/A");
    Eigen::VectorXi B_r = H5Easy::load<Eigen::VectorXi>(file, "/path/to/B");

    BOOST_CHECK_EQUAL(A.isApprox(A_r), true);
    BOOST_CHECK_EQUAL(B.isApprox(B_r), true);
}
#endif

#ifdef H5_USE_EIGEN
BOOST_AUTO_TEST_CASE(H5Easy_Eigen_MatrixXRowMajor)
{
    typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> MatrixXd;
    typedef Eigen::Matrix<int   , Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> MatrixXi;

    H5Easy::File file("test.h5", H5Easy::File::Overwrite);

    MatrixXd A = 100. * MatrixXd::Random(20, 5);
    MatrixXi B = A.cast<int>();

    H5Easy::dump(file, "/path/to/A", A);
    H5Easy::dump(file, "/path/to/B", B);

    MatrixXd A_r = H5Easy::load<MatrixXd>(file, "/path/to/A");
    MatrixXi B_r = H5Easy::load<MatrixXi>(file, "/path/to/B");

    BOOST_CHECK_EQUAL(A.isApprox(A_r), true);
    BOOST_CHECK_EQUAL(B.isApprox(B_r), true);
}
#endif

#ifdef H5_USE_EIGEN
BOOST_AUTO_TEST_CASE(H5Easy_Eigen_VectorXRowMajor)
{
    typedef Eigen::Matrix<double, 1, Eigen::Dynamic, Eigen::RowMajor> VectorXd;
    typedef Eigen::Matrix<int   , 1, Eigen::Dynamic, Eigen::RowMajor> VectorXi;

    H5Easy::File file("test.h5", H5Easy::File::Overwrite);

    VectorXd A = 100. * VectorXd::Random(20);
    VectorXi B = A.cast<int>();

    H5Easy::dump(file, "/path/to/A", A);
    H5Easy::dump(file, "/path/to/B", B);

    VectorXd A_r = H5Easy::load<VectorXd>(file, "/path/to/A");
    VectorXi B_r = H5Easy::load<VectorXi>(file, "/path/to/B");

    BOOST_CHECK_EQUAL(A.isApprox(A_r), true);
    BOOST_CHECK_EQUAL(B.isApprox(B_r), true);
}
#endif
