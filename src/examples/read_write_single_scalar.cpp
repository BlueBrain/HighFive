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

const std::string file_name("read_write_scalar.h5");
const std::string dataset_name("single_scalar");

// Create a dataset name "single_scalar"
// which contains only the perfect integer number "42"
//
int main(void) {
    using namespace HighFive;

    // Create a new file using the default property lists.
    File file(file_name, File::ReadWrite | File::Create | File::Truncate);

    int perfect_number = 42;

    // Create the dataset
    DataSet dataset = file.createDataSet<double>(dataset_name, DataSpace::From(perfect_number));

    // write it
    dataset.write(perfect_number);

    // flush everything
    file.flush();

    // let's read it back
    int potentially_perfect_number;

    dataset.read(potentially_perfect_number);

    std::cout << "perfect number: " << potentially_perfect_number << std::endl;

    return 0;  // successfully terminated
}
