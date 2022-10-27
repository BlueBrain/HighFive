/*
 *  Copyright (c), 2017, Adrien Devresse
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

const std::string FILE_NAME("parallel_dataset_example.h5");
const std::string DATASET_NAME("dset");

//
// simple example to write a dataset with Parallel HDF5 with MPI-IO
//
// The dataset is written from several MPI node in parallel
//
//
int main(int argc, char** argv) {
    int mpi_rank, mpi_size;

    // initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    using namespace HighFive;
    try {
        // open a new file with the MPI IO driver for parallel Read/Write
        File file(FILE_NAME,
                  File::ReadWrite | File::Create | File::Truncate,
                  MPIOFileDriver(MPI_COMM_WORLD, MPI_INFO_NULL));

        // we define the size of our dataset to
        //  lines : total number of mpi_rank
        //  columns : 2
        std::vector<size_t> dims(2);
        dims[0] = std::size_t(mpi_size);
        dims[1] = 2;

        // Create the dataset
        DataSet dataset = file.createDataSet<double>(DATASET_NAME, DataSpace(dims));

        // Each node want to write its own rank two time in
        // its associated row
        double data[1][2] = {{mpi_rank * 1.0, mpi_rank * 2.0}};

        auto xfer_props = DataTransferProps{};
        xfer_props.add(UseCollectiveIO{});

        // write it to the associated mpi_rank
        dataset.select({std::size_t(mpi_rank), 0}, {1, 2}).write(data, xfer_props);

        // Currently, HighFive doesn't wrap retrieving information from property lists.
        // Therefore, one needs to use HDF5 directly. For example, so see if and why
        // collective MPI-IO operations were used, one may:
        uint32_t local_cause = 0, global_cause = 0;
        auto err = H5Pget_mpio_no_collective_cause(xfer_props.getId(), &local_cause, &global_cause);
        if (err < 0) {
            throw std::runtime_error("Failed to check mpio_no_collective_cause.");
        }
        if (local_cause || global_cause) {
            std::cout << "The operation wasn't collective: " << local_cause << " " << global_cause
                      << std::endl;
            throw std::runtime_error("IO wasn't collective.");
        } else {
            std::cout << "Success! The operation was collective.\n";
        }


    } catch (Exception& err) {
        // catch and print any HDF5 error
        std::cerr << err.what() << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Finalize();
    return 0;  // successfully terminated
}
