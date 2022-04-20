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

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include "tests_high_five.hpp"

using namespace HighFive;

struct MpiFixture {
    MpiFixture(int argc, char** argv) {
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
    }
    ~MpiFixture() {
        MPI_Finalize();
    }

    int rank;
    int size;
};

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
    FileDriver adam;
    adam.add(MPIOFileAccess(MPI_COMM_WORLD, MPI_INFO_NULL));
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate, adam);

    DataSet dataset = file.createDataSet<T>(DATASET_NAME, DataSpace::From(values));

    dataset.write(values);

    file.flush();

    // read it back
    Vector result;
    std::vector<size_t> offset;
    offset.push_back(offset_x);
    std::vector<size_t> size;
    size.push_back(count_x);

    Selection slice = dataset.select(offset, size);

    CHECK(slice.getSpace().getDimensions()[0] == size_x);
    CHECK(slice.getMemSpace().getDimensions()[0] == count_x);

    slice.read(result);

    CHECK(result.size() == count_x);

    for (size_t i = offset_x; i < count_x; ++i) {
        CHECK(values[i + offset_x] == result[i]);
    }
}

TEMPLATE_LIST_TEST_CASE("mpiSelectionArraySimple", "[template]", numerical_test_types) {
    selectionArraySimpleTestParallel<TestType>();
}

int main(int argc, char* argv[]) {
    MpiFixture mpi(argc, argv);

    // Capture stdout along solutions in
    // https://stackoverflow.com/questions/58289895/is-it-possible-to-use-catch2-for-testing-an-mpi-code
    std::stringstream ss;
    auto cout_buf = std::cout.rdbuf(ss.rdbuf());
    int result = Catch::Session().run(argc, argv);
    std::cout.rdbuf(cout_buf);

    for (int i = mpi.size - 1; i > 0; --i) {
        MPI_Barrier(MPI_COMM_WORLD);
        if (i == mpi.rank && ss.str().rfind("All tests passed") == std::string::npos) {
            std::cout << ss.str();
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (mpi.rank == 0) {
        std::cout << ss.str();
    }

    return result;
}
