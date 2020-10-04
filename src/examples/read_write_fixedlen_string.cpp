/*
 *  Copyright (c), 2020, Blue Brain Project
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include <iostream>
#include <string>
#include <vector>

#include <highfive/H5File.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>

using namespace HighFive;

static const std::string FILE_NAME("create_dataset_string_example.h5");

// create a dataset from a vector of string
// read it back and print it
int main(void) {

    try {
        // Create a new file using the default property lists.
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);
        std::vector<FixedLengthString<16>> strings_fixed = {{fromString<16>("abcabcabcabcabc")}, {fromString<16>("123123123123123")}};

        // create a dataset ready to contains strings of the size of the vector
        file.createDataSet<FixedLengthString<10>>("ds1", DataSpace(2)).write(strings_fixed);

        // Without specific type info this will create an int8 dataset
        file.createDataSet("ds2", strings_fixed);
    } catch (const Exception& err) {
        // catch and print any HDF5 error
        std::cerr << err.what() << std::endl;
    }

    return 0; // successfully terminated
}
