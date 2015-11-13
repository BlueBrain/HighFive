#include <iostream>
#include <string>
#include <vector>

#include <highfive/H5File.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5DataSet.hpp>


using namespace HighFive;

const std::string FILE_NAME("dataset_integer.h5");
const std::string DATASET_NAME("dset");
const size_t size_dataset = 20;


// create a dataset 1D from a vector of string
void write_dataset(){
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

    std::vector<int> data(size_dataset);
    for(size_t i =0; i < data.size(); ++i){
        data[i] = i;
    }

    DataSet dataset = file.createDataSet< std::vector<int> >(DATASET_NAME,  data);

    // lets write our vector of int to the HDF5 dataset
    dataset.write(data);
}

// read our data back
void read_dataset(){
    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadOnly);

    std::vector<int> read_data;

    DataSet dataset = file.getDataSet(DATASET_NAME);

    // lets write our vector of int
    dataset.read(read_data);

    for(size_t i=0; i < read_data.size(); ++i){
        std::cout << read_data[i] << " ";
    }
}

int main (void)
{

    try
    {
        write_dataset();
        read_dataset();

    }catch(Exception & err){
        // catch and print any HDF5 error
        std::cerr << err.what() << std::endl;
    }

    return 0;  // successfully terminated
}


