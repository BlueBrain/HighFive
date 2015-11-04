#ifndef H5DATASPACE_MISC_HPP
#define H5DATASPACE_MISC_HPP

#include "../H5DataSpace.hpp"
#include "../H5Exception.hpp"

#include <vector>

#include <H5Spublic.h>

namespace HighFive{

DataSpace::DataSpace(const std::vector<size_t> & dims){
    std::vector<hsize_t> real_dims(dims.size());
    std::copy(dims.begin(), dims.end(), real_dims.begin());

    if( (_hid = H5Screate_simple(dims.size(), &(real_dims.at(0)), NULL) ) < 0){
        throw DataSpaceException("Impossible to create DataSpace");
    }
}

DataSpace::DataSpace() {};

size_t DataSpace::getDims() const{
    const int dims = H5Sget_simple_extent_ndims(_hid);
    if(dims < 0){
        HDF5ErrMapper::ToException<DataSetException>("Unable to get DataSpace dimensions");
    }
    return size_t(dims);
}

}


#endif // H5DATASPACE_MISC_HPP
