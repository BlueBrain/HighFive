/*
 *  Copyright (c), 2017-2019, Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#include <string>
#include <iostream>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>


#ifdef H5_USE_BOOST
#include <boost/multi_array.hpp>
#endif

#define BOOST_TEST_MAIN HighFiveTestMultiDims
#include <boost/test/unit_test.hpp>

#include "tests_high_five.hpp"

using namespace HighFive;


/// \brief Test for 2D old-style arrays (T array[x][y])
template <typename T>
void readWrite2DArrayTest() {
    std::ostringstream filename;
    filename << "h5_rw_2d_array_" << typeNameHelper<T>() << "_test.h5";
    const std::string DATASET_NAME("dset");
    const size_t x_size = 100;
    const size_t y_size = 10;

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    // Create the data space for the dataset.
    std::vector<size_t> dims{x_size, y_size};

    DataSpace dataspace(dims);

    // Create a dataset with arbitrary type
    DataSet dataset = file.createDataSet<T>(DATASET_NAME, dataspace);

    T array[x_size][y_size];

    ContentGenerate<T> generator;
    generate2D(array, x_size, y_size, generator);

    dataset.write(array);

    T result[x_size][y_size];
    dataset.read(result);

    for (size_t i = 0; i < x_size; ++i) {
        for (size_t j = 0; j < y_size; ++j) {
            BOOST_CHECK_EQUAL(result[i][j], array[i][j]);
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ReadWrite2DArray, T, numerical_test_types) {
    readWrite2DArrayTest<T>();
}

template <typename T>
void readWriteArrayTest() {
    const size_t x_size = 200;
    typename std::array<T, x_size> vec;
    ContentGenerate<T> generator;
    std::generate(vec.begin(), vec.end(), generator);

    typename std::array<T, x_size> result;
    auto dataset = readWriteDataset<T>(vec, result, 1, "std-array");

    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), vec.begin(), vec.end());

    typename std::array<T, 1> tooSmall;
    BOOST_CHECK_THROW(dataset.read(tooSmall), DataSpaceException);
}
BOOST_AUTO_TEST_CASE_TEMPLATE(readWriteArray, T, numerical_test_types) {
    readWriteArrayTest<T>();
}


template <typename T, typename VectorSubT>
void readWriteVectorNDTest(std::vector<VectorSubT>& ndvec,
                           const std::vector<size_t>& dims) {
    fillVec(ndvec, dims, ContentGenerate<T>());

    std::vector<VectorSubT> result;
    readWriteDataset<T>(ndvec, result, dims.size(), "vector");

    BOOST_CHECK(checkLength(result, dims));
    BOOST_CHECK(ndvec == result);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(readWritSimpleVector, T, numerical_test_types) {
    std::vector<T> vec;
    readWriteVectorNDTest<T>(vec, {50});
}

BOOST_AUTO_TEST_CASE_TEMPLATE(readWrite2DVector, T, numerical_test_types) {
    std::vector<std::vector<T>> _2dvec;
    readWriteVectorNDTest<T>(_2dvec, {10, 8});
}

BOOST_AUTO_TEST_CASE_TEMPLATE(readWrite3DVector, T, numerical_test_types) {
    std::vector<std::vector<std::vector<T>>> _3dvec;
    readWriteVectorNDTest<T>(_3dvec, {10, 5, 4});
}

BOOST_AUTO_TEST_CASE_TEMPLATE(readWrite4DVector, T, numerical_test_types) {
    std::vector<std::vector<std::vector<std::vector<T>>>> _4dvec;
    readWriteVectorNDTest<T>(_4dvec, {5, 4, 3, 2});
}



#ifdef H5_USE_BOOST

template <typename T>
void MultiArray3DTest() {
    typedef typename boost::multi_array<T, 3> MultiArray;

    std::ostringstream filename;
    filename << "h5_rw_multiarray_" << typeNameHelper<T>() << "_test.h5";

    const int size_x = 10, size_y = 10, size_z = 10;
    const std::string DATASET_NAME("dset");
    MultiArray array(boost::extents[size_x][size_y][size_z]);

    ContentGenerate<T> generator;
    std::generate(array.data(), array.data() + array.num_elements(), generator);

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    DataSet dataset = file.createDataSet<T>(DATASET_NAME,
                                            DataSpace::From(array));

    dataset.write(array);

    // read it back
    MultiArray result;

    dataset.read(result);

    for (long i = 0; i < size_x; ++i) {
        for (long j = 0; j < size_y; ++j) {
            for (long k = 0; k < size_z; ++k) {
                BOOST_CHECK_EQUAL(array[i][j][k], result[i][j][k]);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(MultiArray3D, T, numerical_test_types) {
    MultiArray3DTest<T>();
}

template <typename T>
void ublas_matrix_Test() {
    using Matrix = boost::numeric::ublas::matrix<T>;

    std::ostringstream filename;
    filename << "h5_rw_multiarray_" << typeNameHelper<T>() << "_test.h5";

    const size_t size_x = 10, size_y = 10;
    const std::string DATASET_NAME("dset");

    Matrix mat(size_x, size_y);

    ContentGenerate<T> generator;
    for (std::size_t i = 0; i < mat.size1(); ++i) {
        for (std::size_t j = 0; j < mat.size2(); ++j) {
            mat(i, j) = generator();
        }
    }

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    DataSet dataset = file.createDataSet<T>(DATASET_NAME, DataSpace::From(mat));

    dataset.write(mat);

    // read it back
    Matrix result;

    dataset.read(result);

    for (size_t i = 0; i < size_x; ++i) {
        for (size_t j = 0; j < size_y; ++j) {
            BOOST_CHECK_EQUAL(mat(i, j), result(i, j));
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ublas_matrix, T, numerical_test_types) {
    ublas_matrix_Test<T>();
}

#endif
