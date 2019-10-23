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
#include <complex>

#include <H5Tpublic.h>


namespace HighFive {

namespace {  // unnamed
inline DataTypeClass convert_type_class(const H5T_class_t& tclass);
inline std::string type_class_string(DataTypeClass);
}


inline DataType::DataType() {}

inline DataTypeClass DataType::getClass() const {
    return convert_type_class(H5Tget_class(_hid));
}

inline size_t DataType::getSize() const {
    return H5Tget_size(_hid);
}

inline bool DataType::operator==(const DataType& other) const {
    return (H5Tequal(_hid, other._hid) > 0);
}

inline bool DataType::operator!=(const DataType& other) const {
    return !(*this == other);
}

inline std::string DataType::string() const {
    return type_class_string(getClass()) + std::to_string(getSize() * 8);
}


// char mapping
template <>
inline AtomicType<char>::AtomicType() {
    _hid = H5Tcopy(H5T_NATIVE_CHAR);
}

template <>
inline AtomicType<signed char>::AtomicType() {
    _hid = H5Tcopy(H5T_NATIVE_CHAR);
}

template <>
inline AtomicType<unsigned char>::AtomicType() {
    _hid = H5Tcopy(H5T_NATIVE_UCHAR);
}

// short mapping
template <>
inline AtomicType<short>::AtomicType() {
    _hid = H5Tcopy(H5T_NATIVE_SHORT);
}

template <>
inline AtomicType<unsigned short>::AtomicType() {
    _hid = H5Tcopy(H5T_NATIVE_USHORT);
}

// integer mapping
template <>
inline AtomicType<int>::AtomicType() {
    _hid = H5Tcopy(H5T_NATIVE_INT);
}

template <>
inline AtomicType<unsigned>::AtomicType() {
    _hid = H5Tcopy(H5T_NATIVE_UINT);
}

// long mapping
template <>
inline AtomicType<long>::AtomicType() {
    _hid = H5Tcopy(H5T_NATIVE_LONG);
}

template <>
inline AtomicType<unsigned long>::AtomicType() {
    _hid = H5Tcopy(H5T_NATIVE_ULONG);
}

// long long mapping
template <>
inline AtomicType<long long>::AtomicType() {
    _hid = H5Tcopy(H5T_NATIVE_LLONG);
}

template <>
inline AtomicType<unsigned long long>::AtomicType() {
    _hid = H5Tcopy(H5T_NATIVE_ULLONG);
}

// float and double mapping
template <>
inline AtomicType<float>::AtomicType() {
    _hid = H5Tcopy(H5T_NATIVE_FLOAT);
}

template <>
inline AtomicType<double>::AtomicType() {
    _hid = H5Tcopy(H5T_NATIVE_DOUBLE);
}

// boolean mapping
template <>
inline AtomicType<bool>::AtomicType() {
    _hid = H5Tcopy(H5T_NATIVE_HBOOL);
}

// std string
template <>
inline AtomicType<std::string>::AtomicType() {
    _hid = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(_hid, H5T_VARIABLE) < 0) {
        HDF5ErrMapper::ToException<DataTypeException>(
            "Unable to define datatype size to variable");
    }
    // define encoding to UTF-8 by default
    H5Tset_cset(_hid, H5T_CSET_UTF8);
}

template <>
inline AtomicType<std::complex<double> >::AtomicType()
{
    static struct ComplexType : public Object
    {
        ComplexType()
        {
            _hid = H5Tcreate(H5T_COMPOUND, sizeof(std::complex<double>));
            // h5py/numpy compatible datatype
            H5Tinsert(_hid, "r", 0, H5T_NATIVE_DOUBLE);
            H5Tinsert(_hid, "i", sizeof(double), H5T_NATIVE_DOUBLE);
        };
    } complexType;
    _hid = H5Tcopy(complexType.getId());
}


namespace {

inline DataTypeClass convert_type_class(const H5T_class_t& tclass) {
    switch(tclass) {
        case H5T_TIME:
            return DataTypeClass::Time;
        case H5T_INTEGER:
            return DataTypeClass::Integer;
        case H5T_FLOAT:
            return DataTypeClass::Float;
        case H5T_STRING:
            return DataTypeClass::String;
        case H5T_BITFIELD:
            return DataTypeClass::BitField;
        case H5T_OPAQUE:
            return DataTypeClass::Opaque;
        case H5T_COMPOUND:
            return DataTypeClass::Compound;
        case H5T_REFERENCE:
            return DataTypeClass::Reference;
        case H5T_ENUM:
            return DataTypeClass::Enum;
        case H5T_VLEN:
            return DataTypeClass::VarLen;
        case H5T_ARRAY:
            return DataTypeClass::Array;
        case H5T_NO_CLASS:
        case H5T_NCLASSES:
        default:
            return DataTypeClass::Invalid;
    }
}


inline std::string type_class_string(DataTypeClass tclass) {
    switch(tclass) {
        case DataTypeClass::Time:
            return "Time";
        case DataTypeClass::Integer:
            return "Integer";
        case DataTypeClass::Float:
            return "Float";
        case DataTypeClass::String:
            return "String";
        case DataTypeClass::BitField:
            return "BitField";
        case DataTypeClass::Opaque:
            return "Opaque";
        case DataTypeClass::Compound:
            return "Compound";
        case DataTypeClass::Reference:
            return "Reference";
        case DataTypeClass::Enum:
            return "Enum";
        case DataTypeClass::VarLen:
            return "Varlen";
        case DataTypeClass::Array:
            return "Array";
        default:
            return "(Invalid)";
    }
}

}  // namespace


}  // namespace HighFive



#endif // H5DATATYPE_MISC_HPP
