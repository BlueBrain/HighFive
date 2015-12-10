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
#ifndef H5DATASPACE_MISC_HPP
#define H5DATASPACE_MISC_HPP


#include <vector>

#include <H5Spublic.h>

#include "../H5DataSpace.hpp"
#include "../H5Exception.hpp"

#include "H5Utils.hpp"




namespace HighFive{


inline DataSpace::DataSpace(const std::vector<size_t> & dims){
    std::vector<hsize_t> real_dims(dims.size());
    std::copy(dims.begin(), dims.end(), real_dims.begin());

    if( (_hid = H5Screate_simple(dims.size(), &(real_dims.at(0)), NULL) ) < 0){
        throw DataSpaceException("Impossible to create dataspace");
    }
}

inline DataSpace::DataSpace(const size_t dim1){
    const hsize_t dims = hsize_t(dim1);
    if( (_hid = H5Screate_simple(1, &dims, NULL) ) < 0){
        throw DataSpaceException("Unable to create dataspace");
    }
}

inline DataSpace::DataSpace() {}


inline DataSpace DataSpace::clone(){
    DataSpace res;
    if ( (res._hid = H5Scopy(_hid)) < 0){
         throw DataSpaceException("Unable to copy dataspace");
    }
    return res;
}

inline size_t DataSpace::getNumberDimensions() const{
    const int ndim = H5Sget_simple_extent_ndims(_hid);
    if(ndim < 0){
        HDF5ErrMapper::ToException<DataSetException>("Unable to get dataspace number of dimensions");
    }
    return size_t(ndim);
}

std::vector<size_t> DataSpace::getDimensions() const{
    std::vector<hsize_t> dims(getNumberDimensions());
    if( H5Sget_simple_extent_dims(_hid, &(dims[0]), NULL) <0){
        HDF5ErrMapper::ToException<DataSetException>("Unable to get dataspace dimensions");
    }

    std::vector<size_t> res(dims.size());
    std::copy(dims.begin(), dims.end(), res.begin());
    return res;
}

template<typename Value>
DataSpace DataSpace::From(const std::vector<Value> & container){
    return DataSpace(details::get_dim_vector<Value>(container));
}

#ifdef H5_USE_BOOST
template<typename Value, std::size_t Dims>
DataSpace DataSpace::From(const boost::multi_array<Value, Dims> & container){
    std::vector<size_t> dims(Dims);
    for(std::size_t i = 0; i < Dims; ++i){
        dims[i] = container.shape()[i];
    }
    return DataSpace(dims);
}
#endif

}


#endif // H5DATASPACE_MISC_HPP
