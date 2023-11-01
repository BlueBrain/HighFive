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

#include <highfive/highfive.hpp>

using namespace HighFive;

// This examples shows how compile time constant strings work.
//
// Note, that as of version 2.8.0., writing `std::string` as fixed-length
// strings there's a simpler API.
int main() {
    // Create a new file using the default property lists.
    File file("create_dataset_string_example.h5", File::Truncate);
    const char strings_fixed[][16] = {"abcabcabcabcabc", "123123123123123"};

    // create a dataset ready to contains strings of the size of the vector
    file.createDataSet<char[10]>("ds1", DataSpace(2)).write(strings_fixed);

    // Without specific type info this will create an int8 dataset
    file.createDataSet("ds2", strings_fixed);

    // Now test the new interface type
    FixedLenStringArray<10> arr{"0000000", "1111111"};
    auto ds = file.createDataSet("ds3", arr);

    // Read back truncating to 4 chars
    FixedLenStringArray<4> array_back;
    ds.read(array_back);
    std::cout << "First item is '" << array_back[0] << "'\n"
              << "Second item is '" << array_back[1] << "'\n";

    return 0;
}
