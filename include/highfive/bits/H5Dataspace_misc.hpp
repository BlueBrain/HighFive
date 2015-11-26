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
        throw DataSpaceException("Impossible to create dataspace");
    }
}

inline DataSpace::DataSpace() {}

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

template<typename Container>
DataSpace DataSpace::From(const Container & container){
    return DataSpace(details::get_dim_vector<Container>(container));
}

}


#endif // H5DATASPACE_MISC_HPP
