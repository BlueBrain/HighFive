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

#include <highfive/highfive.hpp>

// Create a dataset name "dset" of double 4x6
int main(void) {
    using namespace HighFive;

    // Create a new file using the default property lists. Note that
    // `File::Truncate` will, if present, truncate the file before opening
    // it for reading and writing.
    File file("create_dataset_example.h5", File::Truncate);

    // Define the size of our dataset: 2x6
    std::vector<size_t> dims{2, 6};

    // Create the dataset
    DataSet dataset = file.createDataSet<double>("dset", DataSpace(dims));

    double data[2][6] = {{1.1, 2.2, 3.3, 4.4, 5.5, 6.6},
                         {11.11, 12.12, 13.13, 14.14, 15.15, 16.16}};

    // write it
    dataset.write(data);


    return 0;  // successfully terminated
}
