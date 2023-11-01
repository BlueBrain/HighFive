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

#include <highfive/highfive.hpp>

const std::string file_name("parallel_independent_example.h5");

// This is an example of how to let MPI ranks read independent parts of the
// HDF5 file.
int main(int argc, char** argv) {
    int mpi_rank, mpi_size;

    // initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    using namespace HighFive;
    try {
        // We perform a preprocessing step, to create a file:
        // {
        //   "g0": { "x": [ 0.0, 0.0, 0.0 ] }
        //   "g1": { "x": [ 1.0, 2.0, 3.0 ] }
        //   "g2": { "x": [ 2.0, 4.0, 6.0 ] }
        //   ...
        // }
        if (mpi_rank == 0) {
            File file(file_name, File::ReadWrite | File::Create | File::Truncate);

            for (int i = 0; i < mpi_size; ++i) {
                std::stringstream group_name;
                group_name << "g" << i;

                // Create a group: `f"g{mpi_rank}"`
                auto group = file.createGroup(group_name.str());

                std::vector<double> x{double(i), 2 * double(i), 3 * double(i)};
                group.createDataSet("x", x);
            }
        }

        // We need to wait for the file to be created, before proceeding with the
        // actual example.
        MPI_Barrier(MPI_COMM_WORLD);

        // The example can start!
        //
        // Let's inform HDF5 that we want MPI-IO. We need a file access property
        // list, and request MPI-IO file access.
        FileAccessProps fapl;
        fapl.add(MPIOFileAccess{MPI_COMM_WORLD, MPI_INFO_NULL});

        // Do not ask for collective metadata I/O reads. You can consider
        // asking for collective metadata writes, (since they must be collective
        // anyway, otherwise MPI ranks might have differing view of how the same
        // HDF5 is internally structured). But here we only read.
        //
        // fapl.add(MPIOCollectiveMetadataWrite{});

        // Now we can create the file as usual.
        File file(file_name, File::ReadOnly, fapl);

        // Note that this operation isn't collective. Each MPI rank is requesting to
        // open a different group.
        std::stringstream dataset_name;
        dataset_name << "g" << mpi_rank << "/x";

        // Again this isn't collective since, different MPI ranks are reading
        // from different datasets.
        auto x = file.getDataSet(dataset_name.str()).read<std::vector<double>>();

        // Let's create some more obviously independent accesses, and explicitely
        // open the intermediate group:
        if (mpi_rank % 2 == 0) {
            std::stringstream other_group_name;
            other_group_name << "g" << (mpi_rank + 1) % mpi_size;
            auto other_group = file.getGroup(other_group_name.str());

            auto y = other_group.getDataSet("x").read<std::vector<double>>();
        }
    } catch (Exception& err) {
        // catch and print any HDF5 error
        std::cerr << err.what() << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Finalize();
    return 0;  // successfully terminated
}
