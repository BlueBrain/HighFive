/*
 *  Copyright (c), 2017, Adrien Devresse
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include <iostream>

#undef H5_USE_BOOST
#define H5_USE_BOOST

#include <boost/multi_array.hpp>
#include <highfive/H5File.hpp>

using namespace HighFive;

// Create a 2D dataset 10x3 of double with boost multi array
// and write it to a file.
int main(void) {
    const int nx = 10;
    const int ny = 3;

    boost::multi_array<double, 2> array(boost::extents[nx][ny]);

    for (int i = 0; i < nx; ++i) {
        for (int j = 0; j < ny; ++j) {
            array[i][j] = double(j + i * ny);
        }
    }

    // We create a new HDF5 file
    File file("boost_multiarray_example.h5", File::Truncate);

    // let's create our dataset of the size of the boost::multi_array
    file.createDataSet("dset", array);

    return 0;
}
