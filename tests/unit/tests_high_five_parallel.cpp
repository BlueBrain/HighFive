/*
 *  Copyright (c), 2017-2019, Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#include <cstdlib>
#include <iostream>
#include <string>
#include <typeinfo>
#include <vector>

#include <highfive/H5File.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5Group.hpp>

#define BOOST_TEST_MAIN HighFiveTestParallel
#include <boost/test/unit_test.hpp>

#include "tests_high_five.hpp"

using namespace HighFive;


static int argc = boost::unit_test::framework::master_test_suite().argc;
static char** argv = boost::unit_test::framework::master_test_suite().argv;

struct MpiFixture {
    MpiFixture() { MPI_Init(&argc, &argv); }
    ~MpiFixture() { MPI_Finalize(); }
};

BOOST_GLOBAL_FIXTURE(MpiFixture)
BOOST_GLOBAL_FIXTURE_END

template <typename T>
void selectionArraySimpleTestParallel() {

    int mpi_rank, mpi_size;
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    typedef typename std::vector<T> Vector;

    std::ostringstream filename;
    filename << "h5_rw_select_parallel_test_" << typeNameHelper<T>() << "_test.h5";

    const auto size_x = static_cast<size_t>(mpi_size);
    const auto offset_x = static_cast<size_t>(mpi_rank);
    const auto count_x = static_cast<size_t>(mpi_size - mpi_rank);

    const std::string DATASET_NAME("dset");

    Vector values(size_x);

    ContentGenerate<T> generator;
    std::generate(values.begin(), values.end(), generator);

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate,
              MPIOFileDriver(MPI_COMM_WORLD, MPI_INFO_NULL));

    DataSet dataset =
        file.createDataSet<T>(DATASET_NAME, DataSpace::From(values));

    dataset.write(values);

    file.flush();

    // read it back
    Vector result;
    std::vector<size_t> offset;
    offset.push_back(offset_x);
    std::vector<size_t> size;
    size.push_back(count_x);

    Selection slice = dataset.select(offset, size);

    BOOST_CHECK_EQUAL(slice.getSpace().getDimensions()[0], size_x);
    BOOST_CHECK_EQUAL(slice.getMemSpace().getDimensions()[0], count_x);

    slice.read(result);

    BOOST_CHECK_EQUAL(result.size(), count_x);

    for (size_t i = offset_x; i < count_x; ++i) {
        // std::cout << result[i] << " ";
        BOOST_CHECK_EQUAL(values[i + offset_x], result[i]);
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(selectionArraySimple, T, numerical_test_types) {

    selectionArraySimpleTestParallel<T>();
}
