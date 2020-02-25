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

#include <H5Ppublic.h>
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

inline bool DataType::isVariableStr() const {
    return H5Tis_variable_str(_hid) > 0;
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


// Calculate the padding required to align an element of a struct
#define _H5_STRUCT_PADDING(current_size, member_size) (((member_size) - (current_size)) % (member_size))

inline void CompoundType::create(size_t size) {
    if (size == 0) {
        size_t current_size = 0, max_type_size = 0;

        // Do a first pass to find the total size of the compound datatype
        for (auto& member: members) {
            size_t member_size = H5Tget_size(member.base_type.getId());
            if (member_size == 0) {
                throw DataTypeException("Cannot get size of DataType with hid: " +
                                        std::to_string(member.base_type.getId()));
            }

            // Set the offset of this member within the struct according to the
            // standard alignment rules
            member.offset = current_size + _H5_STRUCT_PADDING(current_size, member_size);

            // Set the current size to the end of the new member
            current_size = member.offset + member_size;

            max_type_size = std::max(max_type_size, member_size);
        }

        size = current_size + _H5_STRUCT_PADDING(current_size, max_type_size);
    }

    // Create the HDF5 type
    if((_hid = H5Tcreate(H5T_COMPOUND, size)) < 0) {
        HDF5ErrMapper::ToException<DataTypeException>(
            "Could not create new compound datatype");
    }

    // Loop over all the members and insert them into the datatype
    for (const auto& member: members) {
        if(H5Tinsert(_hid, member.name.c_str(), member.offset, member.base_type.getId()) < 0) {
            HDF5ErrMapper::ToException<DataTypeException>(
                "Could not add new member to datatype"
            );
        }
    }
}

#undef _H5_STRUCT_PADDING

inline void CompoundType::commit(const Object& object, const std::string& name) const {
    H5Tcommit2(object.getId(), name.c_str(), getId(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
}

template<typename T>
inline void EnumType<T>::create() {
    // Create the HDF5 type
    if((_hid = H5Tenum_create(AtomicType<typename std::underlying_type<T>::type>{}.getId())) < 0) {
        HDF5ErrMapper::ToException<DataTypeException>(
            "Could not create new enum datatype");
    }

    // Loop over all the members and insert them into the datatype
    for (const auto& member: members) {
        if(H5Tenum_insert(_hid, member.name.c_str(), &(member.value)) < 0) {
            HDF5ErrMapper::ToException<DataTypeException>(
                "Could not add new member to this enum datatype"
            );
        }
    }
}

template<typename T>
inline void EnumType<T>::commit(const Object& object, const std::string& name) const {
    H5Tcommit2(object.getId(), name.c_str(), getId(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
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


/// \brief Create a DataType instance representing type T
template <typename T>
inline DataType create_datatype() {
    return AtomicType<T>();
}


/// \brief Create a DataType instance representing type T and perform a sanity check on its size
template <typename T>
inline DataType create_and_check_datatype() {

    DataType t = create_datatype<T>();

    // Check that the size of the template type matches the size that HDF5 is
    // expecting. Skip this check if the base type is a variable length string
    if(!t.isVariableStr() && (sizeof(T) != t.getSize())) {
        std::ostringstream ss;
        ss << "Size of array type " << sizeof(T)
           << " != that of memory datatype " << t.getSize()
           << std::endl;
        throw DataTypeException(ss.str());
    }

    return t;
}

}  // namespace HighFive


#endif // H5DATATYPE_MISC_HPP
