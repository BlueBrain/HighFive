#include <iostream>
#include <string>
#include <vector>

#include <highfive/H5File.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5DataSet.hpp>


using namespace HighFive;

const std::string FILE_NAME("dataset_string.h5");
const std::string DATASET_NAME("story");

// create a dataset from a vector of string
// read it back and print it
int main (void)
{

    try
    {
        // Create a new file using the default property lists.
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

        std::vector<std::string> string_list;
        string_list.push_back("Hello World !");
        string_list.push_back("This string list is mapped to a dataset of variable length string");
        string_list.push_back("Encoding is done in UTF-8 - 你好 - Здравствуйте!");
        string_list.push_back("May the force be with you");
        string_list.push_back("Enjoy !");


        DataSet dataset = file.createDataSet< std::vector<std::string> >(DATASET_NAME,  string_list);

        // lets write our vector of int
        dataset.write(string_list);



        std::vector<std::string> result_string_list;
        // lets write our vector of int
        dataset.read(result_string_list);

        for(int i=0; i < result_string_list.size(); ++i){
            std::cout << ":" << i << " " << result_string_list[i] << "\n";
        }

    }catch(Exception & err){
        // catch and print any HDF5 error
        std::cerr << err.what() << std::endl;
    }

    return 0;  // successfully terminated
}



