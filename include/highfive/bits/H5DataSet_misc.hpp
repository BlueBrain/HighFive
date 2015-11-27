#ifndef H5DATASET_MISC_HPP
#define H5DATASET_MISC_HPP

#include "../H5DataSet.hpp"

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

#include "../H5DataType.hpp"
#include "../H5DataSpace.hpp"

#include "H5Utils.hpp"




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



}

#endif // H5DATASET_MISC_HPP
