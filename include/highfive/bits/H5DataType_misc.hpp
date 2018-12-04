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

#include "../H5DataType.hpp"
#include "../H5Exception.hpp"

#include <H5Tpublic.h>

namespace HighFive {

inline DataType::DataType() {}

inline bool DataType::operator==(const DataType& other) const {
    return (H5Tequal(_hid, other._hid) > 0);
}

inline bool DataType::operator!=(const DataType& other) const {
    return !(*this == other);
}

inline size_t DataType::getSize() const {
    if(!isValid()) {
        throw DataTypeException("Cannot get size of a DataType before creation\
        (try calling autoCreate/manualCreate if this is a CompoundType).");
    }
    return H5Tget_size(_hid);
}

inline bool DataType::isVariableStr() const {
    if(!isValid()) {
        throw DataTypeException("Cannot test a DataType before creation\
        (try calling autoCreate/manualCreate if this is a CompoundType).");
    }
    return (H5Tis_variable_str(_hid) > 0);
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
inline AtomicType<unsigned int>::AtomicType() {
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
#define _STRUCT_PADDING(current_size, member_size) (((member_size) - (current_size)) % (member_size))

inline void CompoundType::addMember(const std::string& name, hid_t base_type, size_t offset=0) {
    member_def t = {name, base_type, offset};
    member_list.push_back(t);
}

inline void CompoundType::addMember(const std::string& name, HighFive::DataType base_type, size_t offset=0) {
    addMember(name, base_type.getId(), offset);
}

inline void CompoundType::autoCreate() {

    size_t current_size = 0, total_size = 0, max_type_size = 0;
    std::vector<member_def>::iterator it;

    // Do a first pass to find the total size of the compound datatype
    for (it = member_list.begin(); it != member_list.end(); it++) {
        size_t member_size;

        member_size = H5Tget_size(it->base_type);

        // Set the offset of this member within the struct according to the
        // standard alignment rules
        it->offset = current_size + _STRUCT_PADDING(current_size, member_size);

        // Set the current size to the end of the new member
        current_size = it->offset + member_size;

        max_type_size = std::max(max_type_size, member_size);
    }

    total_size = current_size + _STRUCT_PADDING(current_size, max_type_size);

    manualCreate(total_size);
}

inline void CompoundType::manualCreate(size_t total_size) {

    hid_t compound_hid;
    std::vector<member_def>::iterator it;

    // Create the HDF5 type
    if((compound_hid = H5Tcreate(H5T_COMPOUND, total_size)) < 0) {
        HDF5ErrMapper::ToException<DataTypeException>(
            "Could not create new compound datatype")
        ;
    }

    // Loop over all the members and insert them into the datatype
    for (it = member_list.begin(); it != member_list.end(); it++) {

        if(H5Tinsert(compound_hid, it->name.c_str(), it->offset, it->base_type) < 0) {
            HDF5ErrMapper::ToException<DataTypeException>(
                "Could not add new member to datatype"
            );
        }
    }

    _hid = H5Tcopy(compound_hid);

}

inline void CompoundType::commit(const Object& object, const std::string& name) {

    H5Tcommit1(object.getId(), name.c_str(), getId());

}


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

}


#endif // H5DATATYPE_MISC_HPP
