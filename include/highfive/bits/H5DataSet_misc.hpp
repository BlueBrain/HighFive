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
#ifndef H5DATASET_MISC_HPP
#define H5DATASET_MISC_HPP



#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include <numeric>

#ifdef H5_USE_BOOST
#include <boost/multi_array.hpp>
#endif

#include <H5Dpublic.h>
#include <H5Ppublic.h>

#include "../H5DataSet.hpp"
#include "../H5DataType.hpp"
#include "../H5DataSpace.hpp"

#include "H5Utils.hpp"
#include "H5Slice_traits_misc.hpp"



namespace HighFive{


inline DataSet::DataSet() {}


inline size_t DataSet::getStorageSize() const{
    return H5Dget_storage_size(_hid);
}


inline DataType DataSet::getDataType() const{
    DataType res;
    res._hid = H5Dget_type(_hid);
    return res;
}


inline DataSpace DataSet::getSpace() const{
    DataSpace space;
    if( (space._hid = H5Dget_space(_hid)) < 0){
        HDF5ErrMapper::ToException<DataSetException>("Unable to get DataSpace out of DataSet");
    }
    return space;
}



inline DataSpace DataSet::getMemSpace() const{
    return getSpace();
}

}

#endif // H5DATASET_MISC_HPP
