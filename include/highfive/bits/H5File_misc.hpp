#ifndef H5FILE_MISC_HPP
#define H5FILE_MISC_HPP


#include "../H5File.hpp"
#include "../H5Exception.hpp"

#include <H5Ppublic.h>
#include <H5Fpublic.h>


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


}

#endif // H5FILE_MISC_HPP
