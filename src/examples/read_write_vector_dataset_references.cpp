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

#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5Reference.hpp>

using namespace HighFive;

const std::string FILE_NAME("dataset_integer.h5");
const std::string SOURCE_INT_DATASET_NAME("source_dateset");
const std::string REFERENCE_DATASET_NAME("reference_dateset");
const size_t size_dataset = 20;

// create a dataset 1D from a vector of string
void write_dataset() {

    // we create a new hdf5 file
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

    // we create a new group
    Group group = file.createGroup("a_group");

    std::vector<int> data(size_dataset);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = int(i);
    }

    // let's create a dataset of native integer with the size of the vector
    // 'data' inside the group
    DataSet dataset = group.createDataSet<int>(SOURCE_INT_DATASET_NAME, DataSpace::From(data));
    dataset.write(data);

    // create a reference to the dateset containing the integers
    Reference ref = Reference(group, dataset);
    std::vector<Reference> ref_container{ref};

    // in similar fashion, we store as dataset the reference that we want
    DataSet ref_set = group.createDataSet<Reference>(REFERENCE_DATASET_NAME, DataSpace::From(ref_container));
    ref_set.write(ref_container);
}

// read our data back
void read_dataset() {
    // we open the existing hdf5 file we created before
    File file(FILE_NAME, File::ReadOnly);

    // we load the group
    Group my_group = file.getGroup("a_group");

    // we load the dataset that contains the reference
    DataSet ref_dataset = my_group.getDataSet(REFERENCE_DATASET_NAME);

    // we load the vector of references
    std::vector<Reference> expected_references;
    ref_dataset.read(expected_references);

    // we use the stored reference and dereference it to gain access in the integers' dataset
    DataSet expected_dataset = expected_references[0].dereference<DataSet>(my_group);

    // as usual, we load the vector with numbers from the extracted dataset
    std::vector<int> read_data;
    expected_dataset.read(read_data);

    // and voila, the payload we excepted
    for (size_t i = 0; i < read_data.size(); ++i) {
        std::cout << read_data[i] << " ";
    }
}

int main(void) {

    try {
        write_dataset();
        read_dataset();

    } catch (Exception &err) {
        // catch and print any HDF5 error
        std::cerr << err.what() << std::endl;
    }

    return 0;// successfully terminated
}