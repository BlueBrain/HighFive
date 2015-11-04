#ifndef H5DATATYPE_MISC_HPP
#define H5DATATYPE_MISC_HPP

#include "../H5DataType.hpp"
#include "../H5Exception.hpp"


#include <H5Tpublic.h>

namespace HighFive{


DataType::DataType(){
}


bool DataType::operator ==(const DataType & other) const{
    return (H5Tequal(_hid, other._hid) > 0);
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



}

#endif // H5DATATYPE_MISC_HPP
