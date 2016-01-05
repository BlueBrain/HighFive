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


const std::string FILE_NAME("create_dataset_example.h5");
const std::string DATASET_NAME("dset");

// Create a dataset name "dset" of double 4x6
//
int main (void)
{
    using namespace HighFive;
    try
    {
        // Create a new file using the default property lists.
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

        // Define the size of our dataset: 2x6
        std::vector<size_t> dims(2);
        dims[0]= 2;
        dims[1]= 6;

        // Create the dataset
        DataSet dataset = file.createDataSet<double>(DATASET_NAME,  DataSpace(dims));


        double data[2][6] = { { 1.1, 2.2, 3.3, 4.4, 5.5, 6.6}, { 11.11, 12.12, 13.13, 14.14, 15.15, 16.16}};

        // write it
        dataset.write(data);

    }catch(Exception & err){
        // catch and print any HDF5 error
        std::cerr << err.what() << std::endl;
    }

    return 0;  // successfully terminated
}

