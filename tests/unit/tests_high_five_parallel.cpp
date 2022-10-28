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

void check_was_collective(const DataTransferProps& xfer_props) {
    uint32_t local_cause = 0, global_cause = 0;
    if (H5Pget_mpio_no_collective_cause(xfer_props.getId(), &local_cause, &global_cause) < 0) {
        throw std::runtime_error("Failed to check mpio_no_collective_cause.");
    }
    CHECK(local_cause == 0);
    CHECK(global_cause == 0);
}

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

    const std::string d1_name("dset1");
    const std::string d2_name("dset2");

    Vector values(size_x);

    ContentGenerate<T> generator;
    std::generate(values.begin(), values.end(), generator);

    // Create a new file using the default property lists.
    FileDriver adam;
    adam.add(MPIOFileAccess(MPI_COMM_WORLD, MPI_INFO_NULL));
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate, adam);

    DataSet d1 = file.createDataSet<T>(d1_name, DataSpace::From(values));
    if (mpi_rank == 0) {
        d1.write(values);
    }

    DataSet d2 = file.createDataSet<T>(d2_name, DataSpace::From(values));
    auto xfer_props = DataTransferProps{};
    xfer_props.add(UseCollectiveIO{});

    {
        auto offset = std::vector<size_t>{static_cast<size_t>(mpi_rank)};
        auto count = std::vector<size_t>{1ul};
        auto slice = d2.select(offset, count);

        auto local_values = Vector(count[0]);
        local_values[0] = values[offset[0]];

        // Write collectively, each MPI rank writes one slab.
        slice.write(local_values, xfer_props);
        check_was_collective(xfer_props);
    }

    file.flush();

    // -- read it back

    auto check_result = [&values, offset_x, count_x](const Vector& result) {
        CHECK(result.size() == count_x);

        for (size_t i = offset_x; i < count_x; ++i) {
            CHECK(values[i + offset_x] == result[i]);
        }
    };

    auto make_slice = [size_x, offset_x, count_x](DataSet& dataset) {
        auto slice = dataset.select(std::vector<size_t>{offset_x}, std::vector<size_t>{count_x});

        CHECK(slice.getSpace().getDimensions()[0] == size_x);
        CHECK(slice.getMemSpace().getDimensions()[0] == count_x);

        return slice;
    };

    auto s1 = make_slice(d1);
    check_result(s1.template read<Vector>());

    auto s2 = make_slice(d2);
    check_result(s2.template read<Vector>(xfer_props));
    check_was_collective(xfer_props);
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
