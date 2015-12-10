/*
 * Copyright (C) 2015 Adrien Devresse <adrien.devresse@epfl.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include <iostream>
#include <string>
#include <vector>

#include <highfive/H5File.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5DataSet.hpp>


using namespace HighFive;

const std::string FILE_NAME("create_dataset_string_example.h5");
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


        // create a dataset ready to contains strings of the size of the vector string_list
        DataSet dataset = file.createDataSet<std::string>(DATASET_NAME,  DataSpace::From(string_list));

        // lets write our vector of  string
        dataset.write(string_list);



        // now we read it back
        std::vector<std::string> result_string_list;
        dataset.read(result_string_list);

        for(size_t i=0; i < result_string_list.size(); ++i){
            std::cout << ":" << i << " " << result_string_list[i] << "\n";
        }

    }catch(Exception & err){
        // catch and print any HDF5 error
        std::cerr << err.what() << std::endl;
    }

    return 0;  // successfully terminated
}



