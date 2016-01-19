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
#ifndef H5FILE_MISC_HPP
#define H5FILE_MISC_HPP


#include "../H5File.hpp"
#include "../H5Exception.hpp"

#include <H5Ppublic.h>
#include <H5Fpublic.h>


namespace HighFive {


namespace{

// libhdf5 uses a preprocessor trick on their oflags
// we can not declare them constant without a mapper
inline int convert_open_flag(int openFlags){
    int res_open = 0;
    if(openFlags & File::ReadOnly)
        res_open |= H5F_ACC_RDONLY;
    if(openFlags & File::ReadWrite)
        res_open |= H5F_ACC_RDWR;
    if(openFlags & File::Create)
        res_open |= H5F_ACC_CREAT;
    if(openFlags & File::Truncate)
        res_open |= H5F_ACC_TRUNC;
    if(openFlags & File::Excl)
        res_open |= H5F_ACC_EXCL;
    return res_open;
}

}


inline File::File(const std::string &filename, int openFlags) : _filename(filename){

    openFlags = convert_open_flag(openFlags);

    if(openFlags & H5F_ACC_CREAT){
        if( (_hid = H5Fcreate(_filename.c_str(), openFlags & (H5F_ACC_TRUNC), H5P_DEFAULT, H5P_DEFAULT)) < 0){
            HDF5ErrMapper::ToException<FileException>(std::string("Unable to create file " + _filename));
        }
    }else{
        if( (_hid = H5Fopen(_filename.c_str(), openFlags, H5P_DEFAULT)) < 0){
            HDF5ErrMapper::ToException<FileException>(std::string("Unable to open file " + _filename));
        }
    }
}


}

#endif // H5FILE_MISC_HPP
