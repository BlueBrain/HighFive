/*
 *   Copyright (c), 2021 Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include <iostream>
#include <string>
#include <vector>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5Reference.hpp>

// create a dataset 1D from a vector of int
void write_dataset() {

    // we create a new hdf5 file
    HighFive::File file("dataset_integer.h5", HighFive::File::Overwrite);

    // we create a new group
    HighFive::Group group = file.createGroup("a_group");

    std::vector<int> data(20);
    std::iota(data.begin(), data.end(), 0);

    // let's create a dataset of native integer with the size of the vector
    // 'data' inside the group
    auto dataset = group.createDataSet("source_dataset", data);

    // create a reference to the dataset containing the integers
    HighFive::Reference ref = HighFive::Reference(group, dataset);
    std::vector<HighFive::Reference> ref_container{ref};

    // in similar fashion, we store as dataset the vector of reference that we want
    HighFive::DataSet ref_set = group.createDataSet("reference_dataset", ref_container);
}

// read our data back
void read_dataset() {
    // we open the existing hdf5 file we created before
    HighFive::File file("dataset_integer.h5", HighFive::File::ReadOnly);

    // we load the group
    HighFive::Group my_group = file.getGroup("a_group");

    // we load the dataset that contains the reference
    HighFive::DataSet ref_dataset = my_group.getDataSet("reference_dataset");

    // we load the vector of references
    std::vector<HighFive::Reference> expected_references;
    ref_dataset.read(expected_references);

    // we use the stored reference and dereference it to gain access in the integers'
    // dataset
    HighFive::DataSet expected_dataset =
        expected_references[0].dereference<HighFive::DataSet>(my_group);

    // as usual, we load the vector with numbers from the extracted dataset
    std::vector<int> read_data;
    expected_dataset.read(read_data);

    // and voila, the payload we excepted
    for (int i : read_data) {
        std::cout << i << " ";
    }
}

int main() {
    try {
        write_dataset();
        read_dataset();

    } catch (const HighFive::Exception& err) {
        // catch and print any HDF5 error
        std::cerr << err.what() << std::endl;
    }
    return 0;  // successfully terminated
}
