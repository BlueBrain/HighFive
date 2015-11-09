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

// determine at compile time number of dimensions of in memory datasets
template<typename T>
struct array_size { static const size_t value = 0; };

template<typename T>
struct array_size<std::vector<T> > { static const size_t value = 1 + array_size<T>::value; };

template<typename T>
struct array_size<T*> { static const size_t value = 1 + array_size<T>::value; };

template<typename T, std::size_t N>
struct array_size<T [N]> { static const size_t value = 1 + array_size<T>::value;  };

// determine at compile time atomic type of the dataset
template<typename T>
struct type_of_array { typedef T type; };

template<typename T>
struct type_of_array<std::vector<T> > { typedef typename type_of_array<T>::type type; };

template<typename T>
struct type_of_array<T*> { typedef typename type_of_array<T>::type type; };

template<typename T, std::size_t N>
struct type_of_array<T [N]> { typedef typename type_of_array<T>::type type; };

// apply conversion operations to the incoming data
template<typename Array>
struct data_converter{
    inline data_converter(Array & datamem){ /* do nothing */ }

    inline Array & transform_read (Array & datamem, const std::vector<size_t> & dims) { (void) dims; return datamem; }

    inline Array & transform_write (Array & datamem) { return datamem; }

    inline void process_result(Array & datamem){ /*do nothing*/ }
};

// apply conversion for vectors 1D
template<typename T>
struct data_converter<std::vector<T> >{
    inline data_converter(std::vector<T> & vec){}

    inline T*  transform_read(std::vector<T> & vec, const std::vector<size_t> & dims) { vec.resize(dims[0]);  return &(vec[0]); }

    inline T*  transform_write(std::vector<T> & vec) { return &(vec[0]); }

    inline void process_result(std::vector<T> & mem){ /*do nothing*/ }

};

// apply conversion for vectors of string (derefence)
template<>
struct data_converter<std::vector<std::string> >{
    inline data_converter(std::vector<std::string> & vec){}

    // create a C vector adapted to HDF5
    // fill last element with NULL to identify end
    inline char**  transform_read(std::vector<std::string> & vec, const std::vector<size_t> & dims) {
        c_vec.resize(dims[0], NULL);
        return (&c_vec[0]);
    }

    static inline char* char_converter(const std::string & str){
        return const_cast<char*>(str.c_str());
    }

    inline char**  transform_write(std::vector<std::string> & vec) {
        c_vec.resize(vec.size()+1, NULL);
        std::transform(vec.begin(), vec.end(), c_vec.begin(), &char_converter);
        return (&c_vec[0]);
    }

    inline void process_result(std::vector<std::string> & vec){
        vec.resize(c_vec.size());
        for(size_t i = 0; i < vec.size(); ++i){
            vec[i] = std::string(c_vec[i]);
            //std::cout << " " << c_vec[i];
        }

    }

    std::vector<char*> c_vec;

};

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
inline void DataSet::read(T & array){
    const size_t dim_array = details::array_size<T>::value;
    const size_t dim_dataset = getSpace().getNumberDimensions();
    if(dim_array != dim_dataset){
        std::ostringstream ss;
        ss << "Impossible to read DataSet of dimensions " << dim_dataset << " into arrays of dimensions " << dim_array;
        throw DataSpaceException(ss.str());
    }

    // Create mem datatype
    const AtomicType<typename details::type_of_array<T>::type > array_datatype;

    // Apply pre read convertions
    details::data_converter<T> converter(array);

    if( H5Dread(_hid, array_datatype.getId(), H5S_ALL, H5S_ALL, H5P_DEFAULT,
                static_cast<void*>(converter.transform_read(array, getSpace().getDimensions() ))) < 0){
         HDF5ErrMapper::ToException<DataSetException>("Error during HDF5 Read: ");
    }

    // re-arrange results
    converter.process_result(array);
}


template <typename T>
inline void DataSet::write(T & buffer){
    const size_t dim_buffer = details::array_size<T>::value;
    const size_t dim_dataset = getSpace().getNumberDimensions();
    if(dim_buffer != dim_dataset){
        std::ostringstream ss;
        ss << "Impossible to write buffer of dimensions " << dim_buffer << " into dataset of dimensions " << dim_dataset;
        throw DataSpaceException(ss.str());
    }

    const AtomicType<typename details::type_of_array<T>::type > array_datatype;

    // Apply pre write convertions
    details::data_converter<T> converter(buffer);

    if( H5Dwrite(_hid, array_datatype.getId(), H5S_ALL, H5S_ALL, H5P_DEFAULT, static_cast<const void*>(converter.transform_write(buffer))) < 0){
         HDF5ErrMapper::ToException<DataSetException>("Error during HDF5 Write: ");
    }
}

}

#endif // H5DATASET_MISC_HPP
