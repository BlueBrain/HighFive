/*
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

#include <highfive/highfive.hpp>
#include <highfive/half_float.hpp>

const std::string FILE_NAME("create_dataset_half_float_example.h5");
const std::string DATASET_NAME("dset");

// Create a dataset name "dset", size 4x6, and type float16_t (i.e., 16-bit half-precision
// floating-point format)
//
int main(void) {
    using namespace HighFive;

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

    // Define the size of our dataset: 4x6
    std::vector<size_t> dims{4, 6};

    // Create the dataset
    DataSet dataset = file.createDataSet<float16_t>(DATASET_NAME, DataSpace(dims));

    std::vector<std::vector<float16_t>> data;
    for (size_t i = 0; i < 4; ++i) {
        data.emplace_back();
        for (size_t j = 0; j < 6; ++j)
            data[i].emplace_back((i + 1) * (j + 1));
    }

    // write it
    dataset.write(data);

    return 0;
}
