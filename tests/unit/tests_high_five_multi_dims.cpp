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

#include <highfive/highfive.hpp>


#ifdef HIGHFIVE_TEST_BOOST
#include <boost/multi_array.hpp>
#include <highfive/boost.hpp>
#endif

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

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
            CHECK(result[i][j] == array[i][j]);
        }
    }
}

TEMPLATE_LIST_TEST_CASE("ReadWrite2DArray", "[template]", numerical_test_types) {
    readWrite2DArrayTest<TestType>();
}

#ifdef HIGHFIVE_TEST_BOOST

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

    DataSet dataset = file.createDataSet<T>(DATASET_NAME, DataSpace::From(array));

    dataset.write(array);

    // read it back
    MultiArray result;

    dataset.read(result);

    for (long i = 0; i < size_x; ++i) {
        for (long j = 0; j < size_y; ++j) {
            for (long k = 0; k < size_z; ++k) {
                CHECK(array[i][j][k] == result[i][j][k]);
            }
        }
    }
}

TEMPLATE_LIST_TEST_CASE("MultiArray3D", "[template]", numerical_test_types) {
    MultiArray3DTest<TestType>();
}

#endif
