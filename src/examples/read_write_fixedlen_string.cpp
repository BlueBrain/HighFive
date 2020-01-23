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

const std::string FILE_NAME("create_dataset_string_example.h5");
const std::string DATASET_NAME("story");

// create a dataset from a vector of string
// read it back and print it
int main(void) {

    try {
        // Create a new file using the default property lists.
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

        //std::vector<std::string> strings_fixed = {"one", "two", "three", "four", "five"};
        char strings_fixed[][4] = {"abc", "123"};

        // create a dataset ready to contains strings of the size of the vector
        // string_list
        //DataSet dataset = file.createDataSet<int>(DATASET_NAME, DataSpace(10));
        DataSet dataset = file.createDataSet(DATASET_NAME, strings_fixed);

        DataSet dataset2 = file.createDataSet<char[10]>("ds2", DataSpace(2));
        dataset2.write(strings_fixed);

        // // now we read it back
        // std::vector<std::string> result_string_list;
        // dataset.read(result_string_list);

        // for (size_t i = 0; i < result_string_list.size(); ++i) {
        //     std::cout << ":" << i << " " << result_string_list[i] << "\n";
        // }

    } catch (Exception& err) {
        // catch and print any HDF5 error
        std::cerr << err.what() << std::endl;
    }

    return 0; // successfully terminated
}
