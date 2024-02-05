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

#include <highfive/highfive.hpp>

const std::string file_name("read_write_raw_ptr.h5");
const std::string dataset_name("array");

// This create a "multi-dimensional" array. Meaning a pointer with
// dimensions. The `std::vector<double>` is mearly a convenient way
// of allocating and releasing memory.
//
// Conceptionually this is only a raw pointer with dimensions. The
// data is store in row-major, aka C-style, without stride, offset
// or padding.
std::vector<double> make_array(const std::vector<size_t>& dims) {
    auto n_elements = dims[0] * dims[1];
    std::vector<double> nd_array(n_elements, 0.0);

    for (size_t i = 0; i < dims[0]; ++i) {
        for (size_t j = 0; j < dims[1]; ++j) {
            nd_array[j + i * dims[1]] = double(j) + 100.0 * double(i);
        }
    }

    return nd_array;
}

int main(void) {
    using namespace HighFive;

    // Create a new file using the default property lists.
    File file(file_name, File::ReadWrite | File::Create | File::Truncate);

    // Let's write to file.
    {
        std::vector<size_t> dims{3, 5};
        auto nd_array = make_array(dims);

        // First, create a dataset with the correct dimensions.
        auto dataset = file.createDataSet<double>(dataset_name, DataSpace(dims));

        // Then write, using the raw pointer.
        dataset.write_raw(nd_array.data());
    }

    // Let's read from file.
    {
        auto dataset = file.getDataSet(dataset_name);

        // First read the dimensions.
        auto dims = dataset.getDimensions();

        // Then allocate memory.
        auto n_elements = dims[0] * dims[1];
        auto nd_array = std::vector<double>(n_elements);

        // Finally, read into the memory by passing a raw pointer to the library.
        dataset.read_raw<double>(nd_array.data());
    }

    return 0;
}
