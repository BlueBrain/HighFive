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

using namespace HighFive;

// create a dataset from a vector of string
// read it back and print it
int main(void) {
    // Create a new file using the default property lists.
    File file("create_attribute.h5", File::Truncate);

    // Create a dummy dataset of one single integer
    DataSet dataset = file.createDataSet("dset", DataSpace(1), create_datatype<int>());

    // Now let's add a attribute on this dataset
    // This attribute will be named "note"
    // and have the following content
    std::string note = "Very important Dataset!";

    // Write in one line of code:
    dataset.createAttribute<std::string>("note", note);

    // We also add a "version" attribute
    // that will be an array 1x2 of integer
    std::vector<int> version{1, 0};

    Attribute v = dataset.createAttribute("version", version);

    // We can also create attributes on the file:
    file.createAttribute("file_version", 1);

    // and on groups in the file:
    auto group = file.createGroup("group");
    group.createAttribute("secret", 123);

    // let's now list the keys of all attributes
    std::vector<std::string> all_attributes_keys = dataset.listAttributeNames();
    for (const auto& attr: all_attributes_keys) {
        std::cout << "attribute: " << attr << std::endl;
    }

    return 0;
}
