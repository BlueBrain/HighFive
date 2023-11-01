/*
 *  Copyright (c), 2017, Adrien Devresse
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <highfive/highfive.hpp>

const std::string file_name("select_partial_string.h5");
const std::string dataset_name("message");

// Create a dataset name "dset" of double 4x6
//
int main(void) {
    using namespace HighFive;

    // Create a new file using the default property lists.
    File file(file_name, File::Truncate);

    {
        // We have a set of string
        std::vector<std::string> values = {
            "Cat",
            "Dog",
            "Hello",
            "Tree",
            "World",
            "Plane",
            ", ",
            "你好",
            "Tea",
            "Moon",
            "صباح جميل",
            "Spaceship",
        };

        // let's create a dataset
        DataSet dataset = file.createDataSet<std::string>(dataset_name, DataSpace::From(values));

        // and write them
        dataset.write(values);
    }

    {
        DataSet dataset = file.getDataSet(dataset_name);

        // now let's read back by cherry pick our interesting string
        std::vector<std::string> result;
        // we select only element N° 2 and 5
        dataset.select(ElementSet({2, 4, 6, 7, 6, 10})).read(result);

        // and display it
        for (auto i: result) {
            std::cout << i << " ";
        }
        std::cout << "\n";
    }

    return 0;  // successfully terminated
}
