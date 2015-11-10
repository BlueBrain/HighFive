#ifndef H5FILE_MISC_HPP
#define H5FILE_MISC_HPP


#include "../H5File.hpp"
#include "../H5Exception.hpp"
#include "../H5DataSet.hpp"
#include "../H5DataSpace.hpp"
#include "../H5DataType.hpp"

#include <H5Dpublic.h>
#include <H5Tpublic.h>
#include <H5Fpublic.h>
#include <H5Ppublic.h>

namespace HighFive {

const int File::ReadOnly = H5F_ACC_RDONLY;

const int File::ReadWrite = H5F_ACC_RDWR;

const int File::Create = H5F_ACC_CREAT;

const int File::Truncate = H5F_ACC_TRUNC;

inline File::File(const std::string &filename, int openFlags) : _filename(filename){

    if(openFlags & H5F_ACC_CREAT){
        if( (_hid = H5Fcreate(_filename.c_str(), openFlags & (H5F_ACC_TRUNC), H5P_DEFAULT, H5P_DEFAULT)) < 0){
            HDF5ErrMapper::ToException<FileException>(std::string("Impossible to create file " + _filename));
        }
    }else{
        if( (_hid = H5Fopen(_filename.c_str(), openFlags, H5P_DEFAULT)) < 0){
            HDF5ErrMapper::ToException<FileException>(std::string("Impossible to open file " + _filename));
        }
    }
}

inline DataSet File::createDataSet(const std::string & dataset_name, const DataSpace & space, const DataType & dtype){
    DataSet set;
    if( (set._hid = H5Dcreate2(_hid, dataset_name.c_str(), dtype._hid, space._hid,
                              H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0){
        HDF5ErrMapper::ToException<DataSetException>(std::string("Unable to create the dataset \"")+ dataset_name+ "\":");
    }
    return set;
}

template <typename Type>
inline DataSet File::createDataSet(const std::string & dataset_name, const DataSpace & space){
    return createDataSet(dataset_name, space, AtomicType<Type>());
}

template <typename Vector>
inline DataSet File::createDataSet(const std::string & dataset_name, const Vector & vector){
    return createDataSet(dataset_name, DataSpace(vector.size()), AtomicType<typename details::type_of_array<Vector>::type>());
}

inline DataSet File::getDataSet(const std::string & dataset_name){
    DataSet set;
    if( (set._hid = H5Dopen2(_hid, dataset_name.c_str(), H5P_DEFAULT)) < 0){
        HDF5ErrMapper::ToException<DataSetException>(std::string("Unable to open the dataset \"")+ dataset_name+ "\":");
    }
    return set;
}

}

#endif // H5FILE_MISC_HPP
