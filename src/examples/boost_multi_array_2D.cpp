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

#undef H5_USE_BOOST
#define H5_USE_BOOST

#include <highfive/H5File.hpp>
#include <boost/multi_array.hpp>

using namespace HighFive;

const std::string FILE_NAME("boost_multiarray_example.h5");
const std::string DATASET_NAME("dset");
const size_t size_x = 10;
const size_t size_y = 3;


// Create a 2D dataset 10x3 of double with boost multi array
// and write it to a file
int main (void)
{

    try
    {

        boost::multi_array<double, 2> my_array(boost::extents[size_x][size_y]);

        for(size_t i = 0; i < size_x; ++i){
            for(size_t j = 0; j < size_y; ++j){
                my_array[i][j]= j + i * size_y;
            }
        }

        // we create a new hdf5 file
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);


        // let's create our dataset of the size of the boost::multi_array
        DataSet dataset = file.createDataSet<double>(DATASET_NAME,  DataSpace::From(my_array));


        // we fill it
        dataset.write(my_array);

    }catch(Exception & err){
        // catch and print any HDF5 error
        std::cerr << err.what() << std::endl;
    }

    return 0;  // successfully terminated
}



