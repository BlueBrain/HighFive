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

#ifdef H5_USE_EIGEN

#include <Eigen/Dense>
#include <highfive/H5File.hpp>

using namespace HighFive;

const std::string FILE_NAME("eigen_matrix_example.h5");
const std::string DATASET_NAME("dset");
const int nrows = 10;
const int ncols = 3;

// Create a 2D dataset 10x3 of double with eigen matrix
// and write it to a file
int main(void) {
    try {
        Eigen::MatrixXd matrix(nrows, ncols);

        for (int i = 0; i < nrows; ++i) {
            for (int j = 0; j < ncols; ++j) {
                matrix(i, j) = double(j + i * 100);
            }
        }

        // we create a new hdf5 file
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

        // let's create our dataset of the size of the eigen matrix
        file.createDataSet(DATASET_NAME, matrix);

    } catch (Exception& err) {
        // catch and print any HDF5 error
        std::cerr << err.what() << std::endl;
    }

    return 0;  // successfully terminated
}

#endif
