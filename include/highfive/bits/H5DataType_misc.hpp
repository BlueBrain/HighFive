/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
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

inline size_t DataType::getSize() const {
    return H5Tget_size(_hid);
}

inline void DataType::setSize(size_t length) const {
    H5Tset_size(_hid, length);
}

inline hid_t DataType::getClass() const {
    return H5Tget_class(_hid);
}

inline bool DataType::isVarLength() const {
    hid_t cls = getClass();
    return cls == H5T_VLEN  ||
          (cls == H5T_STRING && H5Tis_variable_str(_hid));
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


// std string Handling
template <>
inline AtomicType<std::string>::AtomicType(){
    _hid = H5Tcopy(H5T_C_S1);
}

inline StringType::StringType( const DataType & other ) {
    _hid = H5Tcopy(other._hid);
    // H5Tset_cset(_hid, H5T_CSET_UTF8);
}

inline StringType::StringType( size_t fixedLengthTo ) {
    /// define encoding to UTF-8 by default
    /// For the moment we keep the original representation.
    /// Its up to the user to know what encoding he has in the std::string
    // H5Tset_cset(_hid, H5T_CSET_UTF8);

    this->setFixedLengthTo(fixedLengthTo);
}

inline void StringType::setFixedLengthTo( size_t fixedLengthTo ) {
    if( H5Tset_size(_hid, fixedLengthTo) <0){
        HDF5ErrMapper::ToException<DataTypeException>("Unable to define datatype size to variable");
    }

    H5Tset_strpad(_hid, (fixedLengthTo==H5T_VARIABLE)? H5T_STR_NULLPAD : H5T_STR_NULLTERM );
}

inline void StringType::setStrPad(PaddingType pad_t) {
    H5Tset_strpad(_hid, (H5T_str_t)pad_t);
}

inline StringType::PaddingType StringType::getStrPad() {
    return (StringType::PaddingType) H5Tget_strpad(_hid);
}

inline char* StringType::prepareRead(std::string & str) {
    if( isVarLength() ) {
        //H5read allocates for us
        return (char*)&_buf;
    } else {
        //We need to reserve space
        str.resize(getSize());
        return _buf = (char*) str.data();
    }
}

inline void StringType::postRead(std::string & str) {
    if( isVarLength() ) {
        str = _buf;
    }
}

inline char* StringType::prepareWrite(std::string & str) {
    return (char*) str.data();
}

inline void StringType::postWrite(std::string & str) {
    (void) str;
}

inline StringType::~StringType() {
    if ( _buf && isVarLength() ) {
        free(_buf);
    }
}




}

#endif // H5DATATYPE_MISC_HPP
