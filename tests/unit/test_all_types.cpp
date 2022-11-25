/*
 *  Copyright (c), 2017-2019, Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include <string>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5Group.hpp>
#include <highfive/H5Reference.hpp>
#include <highfive/H5Utility.hpp>

#include <catch2/catch_template_test_macros.hpp>

#include "tests_high_five.hpp"

using namespace HighFive;

TEMPLATE_TEST_CASE("Scalar in DataSet",
                   "[Types]",
                   char,
                   signed char,
                   unsigned char,
                   short,
                   unsigned short,
                   int,
                   unsigned,
                   long,
                   unsigned long,
                   long long,
                   unsigned long long,
                   float,
                   double,
                   long double,
                   bool,
                   std::string,
                   std::complex<float>,
                   std::complex<double>,
                   std::complex<long double>) {
    const std::string FILE_NAME("Test_type.h5");
    const std::string DATASET_NAME("dset");
    TestType t1{};

    {
        // Create a new file using the default property lists.
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

        // Create the dataset
        DataSet dataset =
            file.createDataSet(DATASET_NAME, DataSpace(1), create_datatype<TestType>());

        // Write into the initial part of the dataset
        dataset.write(t1);
    }

    // read it back
    {
        File file(FILE_NAME, File::ReadOnly);

        TestType value;
        DataSet dataset = file.getDataSet("/" + DATASET_NAME);
        dataset.read(value);
        CHECK(t1 == value);
    }
}

TEMPLATE_PRODUCT_TEST_CASE("Scalar in std::vector",
                           "[Types]",
                           std::vector,
                           (char,
                            signed char,
                            unsigned char,
                            short,
                            unsigned short,
                            int,
                            unsigned,
                            long,
                            unsigned long,
                            long long,
                            unsigned long long,
                            float,
                            double,
                            long double,
                            /* bool, */
                            std::string,
                            std::complex<float>,
                            std::complex<double>,
                            std::complex<long double>) ) {
    const std::string FILE_NAME("Test_vector.h5");
    const std::string DATASET_NAME("dset");
    TestType t1(5);

    {
        // Create a new file using the default property lists.
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

        // Create the dataset
        DataSet dataset =
            file.createDataSet(DATASET_NAME, {5}, create_datatype<typename TestType::value_type>());

        // Write into the initial part of the dataset
        dataset.write(t1);
    }

    // read it back
    {
        File file(FILE_NAME, File::ReadOnly);

        TestType value;
        DataSet dataset = file.getDataSet("/" + DATASET_NAME);
        dataset.read(value);
        CHECK(t1 == value);
        CHECK(value.size() == 5);
    }
}

#ifdef H5_USE_EIGEN
TEMPLATE_PRODUCT_TEST_CASE("Eigen in std::vector",
                           "[Types]",
                           std::vector,
                           (Eigen::Vector2d, Eigen::Vector3d)) {
    const std::string FILE_NAME("Test_vector.h5");
    const std::string DATASET_NAME("dset");
    TestType t1(5);

    {
        // Create a new file using the default property lists.
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

        // Create the dataset
        DataSet dataset = file.createDataSet(DATASET_NAME, t1);
    }

    // read it back
    {
        File file(FILE_NAME, File::ReadOnly);

        TestType value;
        DataSet dataset = file.getDataSet("/" + DATASET_NAME);
        dataset.read(value);
        CHECK(t1 == value);
        CHECK(value.size() == 5);
    }
}
#endif
