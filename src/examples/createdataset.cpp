#include <iostream>
#include <string>
#include <vector>

#include <highfive/H5File.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5DataSet.hpp>


const std::string FILE_NAME("h5tutr_dset.h5");
const std::string DATASET_NAME("dset");

int main (void)
{
    using namespace HighFive;
    // Try block to detect exceptions raised by any of the calls inside it
    try
    {
        // Create a new file using the default property lists.
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

        // Create the data space for the dataset.
        std::vector<size_t> dims;
        dims.push_back(4);
        dims.push_back(6);

        DataSpace dataspace(dims);

        DataSet dataset = file.createDataSet<double>(DATASET_NAME,  dataspace);

    }catch(Exception & err){
        std::cerr << err.what() << std::endl;
    }

    return 0;  // successfully terminated
}

