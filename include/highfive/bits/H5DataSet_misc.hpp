#ifndef H5DATASET_MISC_HPP
#define H5DATASET_MISC_HPP

#include "../H5DataSet.hpp"

#include <string>
#include <sstream>

#include <H5Dpublic.h>
#include <H5Ppublic.h>

#include "../H5DataType.hpp"
#include "../H5DataSpace.hpp"




namespace HighFive{

namespace details{

// determine at compile time number of dimensions of in memory datasets
template<typename T>
struct array_dims { static const size_t value = 0; };

template<typename T>
struct array_dims<std::vector<T> > { static const size_t value = 1 + array_dims<T>::value; };

template<typename T>
struct array_dims<T*> { static const size_t value = 1 + array_dims<T>::value; };

template<typename T, std::size_t N>
struct array_dims<T [N]> { static const size_t value = 1 + array_dims<T>::value;  };

// determine recursively the size of each dimension of a N dimension vector
template<typename T>
void get_dim_vector_rec(const T & vec, std::vector<size_t> & dims){
    (void) dims;
    (void) vec;
}

template<typename T>
void get_dim_vector_rec(const std::vector<T> & vec, std::vector<size_t> & dims){
    dims.push_back(vec.size());
    get_dim_vector_rec(vec[0], dims);
}

template<typename T>
std::vector<size_t> get_dim_vector(const T & vec){
    std::vector<size_t> dims;
    get_dim_vector_rec(vec, dims);
    return dims;
}



// determine at compile time recursively the basic type of the data
template<typename T>
struct type_of_array { typedef T type; };

template<typename T>
struct type_of_array<std::vector<T> > { typedef typename type_of_array<T>::type type; };

template<typename T>
struct type_of_array<T*> { typedef typename type_of_array<T>::type type; };

template<typename T, std::size_t N>
struct type_of_array<T [N]> { typedef typename type_of_array<T>::type type; };


// same type compile time check
template<typename T, typename U>
struct is_same{
    static const bool value = false;
};

template<typename T>
struct is_same<T, T>{
    static const bool value = true;
};

// hdf5 C pointer type of a C++ collection
template<typename T>
struct array_ptr{
    typedef T type;
};

template<typename T>
struct array_ptr<std::vector<T> >{
    typedef typename array_ptr<T>::type* type;
};

// enable if implem for not c++11 compiler
template <bool Cond, typename T = void>
struct enable_if {};

template <typename T>
struct enable_if<true, T> { typedef T type; };



// apply conversion operations to the incoming data
template<typename Array, class Enable = void>
struct data_converter{
    inline data_converter(Array & datamem, DataSpace & space){
        (void) datamem; (void) space; // do nothing
    }

    inline Array & transform_read (Array & datamem) { return datamem; }

    inline Array & transform_write (Array & datamem) { return datamem; }

    inline void process_result(Array & datamem){
        (void) datamem; // do nothing
    }
};

// apply conversion for vectors 1D
template<typename T >
struct data_converter<std::vector<T>, typename enable_if< (is_same<T, typename type_of_array<T>::type>::value) >::type > {
    inline data_converter(std::vector< T > & vec, DataSpace & space) : _space(space){
        (void) vec;
    }

    inline typename array_ptr<std::vector<T> >::type  transform_read(std::vector<T> & vec) {
            vec.resize(_space.getDimensions()[0]);
            return &(vec[0]);
    }

    inline typename array_ptr<std::vector<T> >::type  transform_write(std::vector<T> & vec) { return &(vec[0]); }

    inline void process_result(std::vector<T> & vec){
         (void) vec;
    }

    DataSpace & _space;
};

// apply conversion for vectors of string (derefence)
template<>
struct data_converter<std::vector<std::string>, void>{
    inline data_converter(std::vector<std::string> & vec, DataSpace & space) : _space(space) {
        (void) vec;
    }

    // create a C vector adapted to HDF5
    // fill last element with NULL to identify end
    inline char**  transform_read(std::vector<std::string> & vec) {
        (void) vec;
        _c_vec.resize(_space.getDimensions()[0], NULL);
        return (&_c_vec[0]);
    }

    static inline char* char_converter(const std::string & str){
        return const_cast<char*>(str.c_str());
    }

    inline char**  transform_write(std::vector<std::string> & vec) {
        _c_vec.resize(vec.size()+1, NULL);
        std::transform(vec.begin(), vec.end(), _c_vec.begin(), &char_converter);
        return (&_c_vec[0]);
    }

    inline void process_result(std::vector<std::string> & vec){
        (void) vec;
        vec.resize(_c_vec.size());
        for(size_t i = 0; i < vec.size(); ++i){
            vec[i] = std::string(_c_vec[i]);
        }

        if(_c_vec.empty() == false && _c_vec[0] != NULL){
             AtomicType<std::string > str_type;
            (void) H5Dvlen_reclaim(str_type.getId(), _space.getId(), H5P_DEFAULT, &(_c_vec[0]));
        }

    }

    std::vector<char*> _c_vec;
    DataSpace &  _space;

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
    const size_t dim_array = details::array_dims<T>::value;
    DataSpace space = getSpace();
    const size_t dim_dataset = space.getNumberDimensions();
    if(dim_array != dim_dataset){
        std::ostringstream ss;
        ss << "Impossible to read DataSet of dimensions " << dim_dataset << " into arrays of dimensions " << dim_array;
        throw DataSpaceException(ss.str());
    }

    // Create mem datatype
    const AtomicType<typename details::type_of_array<T>::type > array_datatype;

    // Apply pre read convertions
    details::data_converter<T> converter(array, space);

    if( H5Dread(_hid, array_datatype.getId(), H5S_ALL, H5S_ALL, H5P_DEFAULT,
                static_cast<void*>(converter.transform_read(array))) < 0){
         HDF5ErrMapper::ToException<DataSetException>("Error during HDF5 Read: ");
    }

    // re-arrange results
    converter.process_result(array);
}


template <typename T>
inline void DataSet::write(T & buffer){
    const size_t dim_buffer = details::array_dims<T>::value;
    DataSpace space = getSpace();
    const size_t dim_dataset = space.getNumberDimensions();
    if(dim_buffer != dim_dataset){
        std::ostringstream ss;
        ss << "Impossible to write buffer of dimensions " << dim_buffer << " into dataset of dimensions " << dim_dataset;
        throw DataSpaceException(ss.str());
    }

    const AtomicType<typename details::type_of_array<T>::type > array_datatype;

    // Apply pre write convertions
    details::data_converter<T> converter(buffer, space);

    if( H5Dwrite(_hid, array_datatype.getId(), H5S_ALL, H5S_ALL, H5P_DEFAULT, static_cast<const void*>(converter.transform_write(buffer))) < 0){
         HDF5ErrMapper::ToException<DataSetException>("Error during HDF5 Write: ");
    }
}

}

#endif // H5DATASET_MISC_HPP
