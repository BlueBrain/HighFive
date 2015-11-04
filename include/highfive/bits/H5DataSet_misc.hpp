#ifndef H5DATASET_MISC_HPP
#define H5DATASET_MISC_HPP

#include "../H5DataSet.hpp"

#include <sstream>

#include <H5Dpublic.h>
#include <H5Ppublic.h>

#include "../H5DataType.hpp"
#include "../H5DataSpace.hpp"




namespace HighFive{

namespace details{

template<typename T>
struct array_size { static const size_t value = 0; };

template<typename T>
struct array_size<T*> { static const size_t value = 1 + array_size<T>::value; };

template<typename T, std::size_t N>
struct array_size<T [N]> { static const size_t value = 1 + array_size<T>::value;  };

template<typename T>
struct type_of_array { typedef T type; };

template<typename T>
struct type_of_array<T*> { typedef typename type_of_array<T>::type type; };

template<typename T, std::size_t N>
struct type_of_array<T [N]> { typedef typename type_of_array<T>::type type; };

}

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


template <typename T>
inline void DataSet::read(T array){
    const size_t dim_array = details::array_size<T>::value;
    const size_t dim_dataset = getSpace().getDims();
    if(dim_array != dim_dataset){
        std::ostringstream ss;
        ss << "Impossible to read DataSet of dimensions " << dim_dataset << " into arrays of dimensions " << dim_array;
        throw DataSpaceException(ss.str());
    }

    const AtomicType<typename details::type_of_array<T>::type > array_datatype;

    if( H5Dread(_hid, array_datatype.getId(), H5S_ALL, H5S_ALL, H5P_DEFAULT, static_cast<void*>(array)) < 0){
         HDF5ErrMapper::ToException<DataSetException>("Error during HDF5 Read: ");
    }
}


template <typename T>
inline void DataSet::write(T buffer){
    const size_t dim_buffer = details::array_size<T>::value;
    const size_t dim_dataset = getSpace().getDims();
    if(dim_buffer != dim_dataset){
        std::ostringstream ss;
        ss << "Impossible to write buffer of dimensions " << dim_buffer << " into dataset of dimensions " << dim_dataset;
        throw DataSpaceException(ss.str());
    }

    const AtomicType<typename details::type_of_array<T>::type > array_datatype;
    if( H5Dwrite(_hid, array_datatype.getId(), H5S_ALL, H5S_ALL, H5P_DEFAULT, static_cast<const void*>(buffer)) < 0){
         HDF5ErrMapper::ToException<DataSetException>("Error during HDF5 Write: ");
    }
}

}

#endif // H5DATASET_MISC_HPP
