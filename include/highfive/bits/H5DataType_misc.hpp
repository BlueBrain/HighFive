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
#ifndef H5DATATYPE_MISC_HPP
#define H5DATATYPE_MISC_HPP

#include <string>

#include "../H5DataType.hpp"
#include "../H5Exception.hpp"


#include <H5Tpublic.h>

namespace HighFive{


inline DataType::DataType(){
}


inline bool DataType::operator ==(const DataType & other) const{
    return (H5Tequal(_hid, other._hid) > 0);
}

inline bool DataType::operator !=(const DataType & other) const{
    return !(*this == other);
}

// char mapping
template <>
inline AtomicType<char>::AtomicType(){
    _hid = H5Tcopy(H5T_NATIVE_CHAR);
}

template <>
inline AtomicType<unsigned char>::AtomicType(){
    _hid = H5Tcopy(H5T_NATIVE_UCHAR);
}

// integer mapping
template <>
inline AtomicType<int>::AtomicType(){
    _hid = H5Tcopy(H5T_NATIVE_INT);
}

template <>
inline AtomicType<unsigned int>::AtomicType(){
    _hid = H5Tcopy(H5T_NATIVE_UINT);
}

// long mapping
template <>
inline AtomicType<long>::AtomicType(){
    _hid = H5Tcopy(H5T_NATIVE_LONG);
}

template <>
inline AtomicType<unsigned long>::AtomicType(){
    _hid = H5Tcopy(H5T_NATIVE_ULONG);
}



// long long mapping
template <>
inline AtomicType<long long>::AtomicType(){
    _hid = H5Tcopy(H5T_NATIVE_LLONG);
}

template <>
inline AtomicType<unsigned long long>::AtomicType(){
    _hid = H5Tcopy(H5T_NATIVE_ULLONG);
}


// float and double mapping
template <>
inline AtomicType<float>::AtomicType(){
    _hid = H5Tcopy(H5T_NATIVE_FLOAT);
}

template <>
inline AtomicType<double>::AtomicType(){
    _hid = H5Tcopy(H5T_NATIVE_DOUBLE);
}

// boolean mapping
template <>
inline AtomicType<bool>::AtomicType(){
    _hid = H5Tcopy(H5T_NATIVE_HBOOL);
}

// std string
template <>
inline AtomicType<std::string>::AtomicType(){
    _hid = H5Tcopy(H5T_C_S1);   
    if( H5Tset_size(_hid,H5T_VARIABLE) <0){
        HDF5ErrMapper::ToException<DataTypeException>("Unable to define datatype size to variable");
    }
    // define encoding to UTF-8 by default
    H5Tset_cset(_hid, H5T_CSET_UTF8);
}

}

#endif // H5DATATYPE_MISC_HPP
