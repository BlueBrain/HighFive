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

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_session.hpp>

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
void selectionArraySimpleTestParallel(File& file) {
    int mpi_rank, mpi_size;
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    using Vector = std::vector<T>;

    const auto size = static_cast<size_t>(mpi_size);
    Vector values(size);

    ContentGenerate<T> generator;
    std::generate(values.begin(), values.end(), generator);

    const std::string d1_name("dset1");
    DataSet d1 = file.createDataSet<T>(d1_name, DataSpace::From(values));
    if (mpi_rank == 0) {
        d1.write(values);
    }

    const std::string d2_name("dset2");
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
    const auto offset = static_cast<size_t>(mpi_rank);
    const auto count = static_cast<size_t>(mpi_size - mpi_rank);

    auto check_result = [&values, offset, count](const Vector& result) {
        CHECK(result.size() == count);

        for (size_t i = offset; i < count; ++i) {
            CHECK(values[i + offset] == result[i]);
        }
    };

    auto make_slice = [size, offset, count](DataSet& dataset) {
        auto slice = dataset.select(std::vector<size_t>{offset}, std::vector<size_t>{count});

        CHECK(slice.getSpace().getDimensions()[0] == size);
        CHECK(slice.getMemSpace().getDimensions()[0] == count);

        return slice;
    };

    auto s1 = make_slice(d1);
    check_result(s1.template read<Vector>());

    auto s2 = make_slice(d2);
    check_result(s2.template read<Vector>(xfer_props));
    check_was_collective(xfer_props);
}

template <typename T>
void selectionArraySimpleTestParallelDefaultProps() {
    std::ostringstream filename;
    filename << "h5_rw_default_props_select_parallel_test_" << typeNameHelper<T>() << "_test.h5";

    // Create a new file using the default property lists.
    auto fapl = FileAccessProps{};
    fapl.add(MPIOFileAccess(MPI_COMM_WORLD, MPI_INFO_NULL));

    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate, fapl);

    selectionArraySimpleTestParallel<T>(file);
}

template <typename T>
void selectionArraySimpleTestParallelCollectiveMDProps() {
    std::ostringstream filename;
    filename << "h5_rw_collective_md_props_select_parallel_test_" << typeNameHelper<T>()
             << "_test.h5";

    // Create a new file using the default property lists.
    auto fapl = FileAccessProps{};
    fapl.add(MPIOFileAccess(MPI_COMM_WORLD, MPI_INFO_NULL));
    fapl.add(MPIOCollectiveMetadata());

    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate, fapl);

    selectionArraySimpleTestParallel<T>(file);
}

TEMPLATE_LIST_TEST_CASE("mpiSelectionArraySimpleDefaultProps", "[template]", numerical_test_types) {
    selectionArraySimpleTestParallelDefaultProps<TestType>();
}

TEMPLATE_LIST_TEST_CASE("mpiSelectionArraySimpleCollectiveMD", "[template]", numerical_test_types) {
    selectionArraySimpleTestParallelCollectiveMDProps<TestType>();
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
