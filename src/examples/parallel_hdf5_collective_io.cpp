/*
 *  Copyright (c), 2017, Adrien Devresse
 *  Copyright (c), 2022, Blue Brain Project
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include <iostream>
#include <string>
#include <vector>

#include <mpi.h>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5PropertyList.hpp>

const std::string FILE_NAME("parallel_collective_example.h5");
const std::string DATASET_NAME("dset");

// Currently, HighFive doesn't wrap retrieving information from property lists.
// Therefore, one needs to use HDF5 directly. For example, to see if collective
// MPI-IO operations were used, one may. Conveniently, this also provides identifiers
// of the cause for not using collective MPI calls.
void check_collective_io(const HighFive::DataTransferProps& xfer_props) {
    uint32_t local_cause = 0, global_cause = 0;
    auto err = H5Pget_mpio_no_collective_cause(xfer_props.getId(), &local_cause, &global_cause);
    if (err < 0) {
        throw std::runtime_error("Failed to check mpio_no_collective_cause.");
    }
    if (local_cause || global_cause) {
        std::cout
            << "The operation was successful, but couldn't use collective MPI-IO. local cause: "
            << local_cause << " global cause:" << global_cause << std::endl;
    }
}


// This is an example of how to write HDF5 files when all
// operations are collective, i.e. all MPI ranks participate in
// all HDF5 related function calls.
//
// If this assumption is met then one can ask HDF5 to use
// collective MPI-IO operations. This enables MPI-IO to optimize
// reads and writes.
//
// In this example we will create groups, and let every MPI rank
// write part of a 2D array; and then have all MPI ranks read back
// a different part of the array.
int main(int argc, char** argv) {
    int mpi_rank, mpi_size;

    // initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    using namespace HighFive;
    try {
        // MPI-IO requires informing HDF5 that we want something other than
        // the default behaviour. This is done through property lists. We
        // need a file access property list.
        FileAccessProps fapl;
        // We tell HDF5 to use MPI-IO
        fapl.add(MPIOFileAccess{MPI_COMM_WORLD, MPI_INFO_NULL});
        // We also specify that we want all meta-data related operations
        // to use MPI collective operations. This implies that all MPI ranks
        // in the communicator must participate in any HDF5 operation that
        // reads or writes metadata. Essentially, this is safe if all MPI ranks
        // participate in all HDF5 operations.
        fapl.add(MPIOCollectiveMetadata{});

        // Now we can create the file as usual.
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate, fapl);

        // We can create a group as usual, but all MPI ranks must participate.
        auto group = file.createGroup("grp");

        // We define the dataset have one row per MPI rank and two columns.
        std::vector<size_t> dims(2);
        dims[0] = std::size_t(mpi_size);
        dims[1] = 2ul;

        // We follow the path for
        DataSet dataset = group.createDataSet<double>(DATASET_NAME, DataSpace(dims));

        // Each node want to write its own rank two time in
        // its associated row
        auto data = std::array<double, 2>{mpi_rank * 1.0, mpi_rank * 2.0};

        auto xfer_props = DataTransferProps{};
        xfer_props.add(UseCollectiveIO{});

        // Each MPI rank writes a non-overlapping part of the array.
        std::vector<size_t> offset{std::size_t(mpi_rank), 0ul};
        std::vector<size_t> count{1ul, 2ul};

        dataset.select(offset, count).write(data, xfer_props);
        check_collective_io(xfer_props);

        // Let's ensure that everything has been written do disk.
        file.flush();

        // We'd like to read back some data. For simplicity, we'll read the
        // row from the MPI above us (wrapping)
        offset[0] = (offset[0] + 1ul) % dims[0];

        // MPI ranks don't have to read non-overlapping parts, but in this
        // example they happen to. Again all rank participate in this call.
        dataset.select(offset, count).read(data, xfer_props);
        check_collective_io(xfer_props);

    } catch (Exception& err) {
        // catch and print any HDF5 error
        std::cerr << err.what() << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Finalize();
    return 0;  // successfully terminated
}
