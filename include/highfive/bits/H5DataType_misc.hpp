/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <string>
#include <complex>
#include <cstring>
#if HIGHFIVE_CXX_STD >= 17
#include <cstddef>
#endif

#include <H5Ppublic.h>

#include "H5Inspector_misc.hpp"
#include "h5t_wrapper.hpp"
#include "h5i_wrapper.hpp"

namespace HighFive {

namespace {  // unnamed
inline DataTypeClass convert_type_class(const H5T_class_t& tclass);
inline std::string type_class_string(DataTypeClass);
inline hid_t create_string(std::size_t length);
}  // namespace

inline bool DataType::empty() const noexcept {
    return _hid == H5I_INVALID_HID;
}

inline DataTypeClass DataType::getClass() const {
    return convert_type_class(detail::h5t_get_class(_hid));
}

inline size_t DataType::getSize() const {
    return detail::h5t_get_size(_hid);
}

inline bool DataType::operator==(const DataType& other) const {
    return detail::h5t_equal(_hid, other._hid) > 0;
}

inline bool DataType::operator!=(const DataType& other) const {
    return !(*this == other);
}

inline bool DataType::isVariableStr() const {
    return detail::h5t_is_variable_str(_hid) > 0;
}

inline bool DataType::isFixedLenStr() const {
    return getClass() == DataTypeClass::String && !isVariableStr();
}

inline bool DataType::isReference() const {
    return detail::h5t_equal(_hid, H5T_STD_REF_OBJ) > 0;
}

inline StringType DataType::asStringType() const {
    if (getClass() != DataTypeClass::String) {
        throw DataTypeException("Invalid conversion to StringType.");
    }

    if (isValid()) {
        detail::h5i_inc_ref(_hid);
    }

    return StringType(_hid);
}

inline std::string DataType::string() const {
    return type_class_string(getClass()) + std::to_string(getSize() * 8);
}

inline StringPadding StringType::getPadding() const {
    return StringPadding(detail::h5t_get_strpad(_hid));
}

inline CharacterSet StringType::getCharacterSet() const {
    return CharacterSet(detail::h5t_get_cset(_hid));
}

inline FixedLengthStringType::FixedLengthStringType(size_t size,
                                                    StringPadding padding,
                                                    CharacterSet character_set) {
    if (size == 0 && padding == StringPadding::NullTerminated) {
        throw DataTypeException(
            "Fixed-length, null-terminated need at least one byte to store the null-character.");
    }

    _hid = detail::h5t_copy(H5T_C_S1);

    detail::h5t_set_size(_hid, hsize_t(size));
    detail::h5t_set_cset(_hid, H5T_cset_t(character_set));
    detail::h5t_set_strpad(_hid, H5T_str_t(padding));
}

inline VariableLengthStringType::VariableLengthStringType(CharacterSet character_set) {
    _hid = detail::h5t_copy(H5T_C_S1);

    detail::h5t_set_size(_hid, H5T_VARIABLE);
    detail::h5t_set_cset(_hid, H5T_cset_t(character_set));
}

// char mapping
template <>
inline AtomicType<char>::AtomicType() {
    _hid = detail::h5t_copy(H5T_NATIVE_CHAR);
}

template <>
inline AtomicType<signed char>::AtomicType() {
    _hid = detail::h5t_copy(H5T_NATIVE_SCHAR);
}

template <>
inline AtomicType<unsigned char>::AtomicType() {
    _hid = detail::h5t_copy(H5T_NATIVE_UCHAR);
}

// short mapping
template <>
inline AtomicType<short>::AtomicType() {
    _hid = detail::h5t_copy(H5T_NATIVE_SHORT);
}

template <>
inline AtomicType<unsigned short>::AtomicType() {
    _hid = detail::h5t_copy(H5T_NATIVE_USHORT);
}

// integer mapping
template <>
inline AtomicType<int>::AtomicType() {
    _hid = detail::h5t_copy(H5T_NATIVE_INT);
}

template <>
inline AtomicType<unsigned>::AtomicType() {
    _hid = detail::h5t_copy(H5T_NATIVE_UINT);
}

// long mapping
template <>
inline AtomicType<long>::AtomicType() {
    _hid = detail::h5t_copy(H5T_NATIVE_LONG);
}

template <>
inline AtomicType<unsigned long>::AtomicType() {
    _hid = detail::h5t_copy(H5T_NATIVE_ULONG);
}

// long long mapping
template <>
inline AtomicType<long long>::AtomicType() {
    _hid = detail::h5t_copy(H5T_NATIVE_LLONG);
}

template <>
inline AtomicType<unsigned long long>::AtomicType() {
    _hid = detail::h5t_copy(H5T_NATIVE_ULLONG);
}

// half-float, float, double and long double mapping
template <>
inline AtomicType<float>::AtomicType() {
    _hid = detail::h5t_copy(H5T_NATIVE_FLOAT);
}

template <>
inline AtomicType<double>::AtomicType() {
    _hid = detail::h5t_copy(H5T_NATIVE_DOUBLE);
}

template <>
inline AtomicType<long double>::AtomicType() {
    _hid = detail::h5t_copy(H5T_NATIVE_LDOUBLE);
}

// std string
template <>
inline AtomicType<std::string>::AtomicType() {
    _hid = create_string(H5T_VARIABLE);
}

#if HIGHFIVE_CXX_STD >= 17
// std byte
template <>
inline AtomicType<std::byte>::AtomicType() {
    _hid = detail::h5t_copy(H5T_NATIVE_B8);
}
#endif

// Fixed-Length strings
// require class specialization templated for the char length
template <size_t StrLen>
class AtomicType<char[StrLen]>: public DataType {
  public:
    inline AtomicType()
        : DataType(create_string(StrLen)) {}
};

template <typename T>
class AtomicType<std::complex<T>>: public DataType {
  public:
    inline AtomicType()
        : DataType(
              CompoundType({{"r", create_datatype<T>(), 0}, {"i", create_datatype<T>(), sizeof(T)}},
                           sizeof(std::complex<T>))) {
        static_assert(std::is_arithmetic<T>::value,
                      "std::complex accepts only floating point and integral numbers.");
    }
};

// For boolean we act as h5py
inline EnumType<details::Boolean> create_enum_boolean() {
    return {{"FALSE", details::Boolean::HighFiveFalse}, {"TRUE", details::Boolean::HighFiveTrue}};
}

// Other cases not supported. Fail early with a user message
template <typename T>
AtomicType<T>::AtomicType() {
    static_assert(
        true,
        "Missing specialization of AtomicType<T>. Therefore, type T is not supported by HighFive.");
}


// Internal
// Reference mapping
template <>
inline AtomicType<Reference>::AtomicType() {
    _hid = detail::h5t_copy(H5T_STD_REF_OBJ);
}

inline size_t find_first_atomic_member_size(hid_t hid) {
    // Recursive exit condition
    if (detail::h5t_get_class(hid) == H5T_COMPOUND) {
        auto number_of_members = detail::h5t_get_nmembers(hid);
        if (number_of_members == -1) {
            throw DataTypeException("Cannot get members of CompoundType with hid: " +
                                    std::to_string(hid));
        }
        if (number_of_members == 0) {
            throw DataTypeException("No members defined for CompoundType with hid: " +
                                    std::to_string(hid));
        }

        auto member_type = detail::h5t_get_member_type(hid, 0);
        auto size = find_first_atomic_member_size(member_type);
        detail::h5t_close(member_type);
        return size;
    } else if (detail::h5t_get_class(hid) == H5T_STRING) {
        return 1;
    }
    return detail::h5t_get_size(hid);
}

// Calculate the padding required to align an element of a struct
// For padding see explanation here: https://en.cppreference.com/w/cpp/language/object#Alignment
// It is to compute padding following last element inserted inside a struct
// 1) We want to push back an element padded to the structure
// 'current_size' is the size of the structure before adding the new element.
// 'member_size' the size of the element we want to add.
// 2) We want to compute the final padding for the global structure
// 'current_size' is the size of the whole structure without final padding
// 'member_size' is the maximum size of all element of the struct
//
// The basic formula is only to know how much we need to add to 'current_size' to fit
// 'member_size'.
// And at the end, we do another computation because the end padding, should fit the biggest
// element of the struct.
//
// As we are with `size_t` element, we need to compute everything inside R+
#define _H5_STRUCT_PADDING(current_size, member_size)                                \
    (((member_size) >= (current_size))                                               \
         ? (((member_size) - (current_size)) % (member_size))                        \
         : ((((member_size) - (((current_size) - (member_size)) % (member_size)))) % \
            (member_size)))

inline void CompoundType::create(size_t size) {
    if (size == 0) {
        size_t current_size = 0, max_atomic_size = 0;

        // Do a first pass to find the total size of the compound datatype
        for (auto& member: members) {
            size_t member_size = detail::h5t_get_size(member.base_type.getId());

            if (member_size == 0) {
                throw DataTypeException("Cannot get size of DataType with hid: " +
                                        std::to_string(member.base_type.getId()));
            }

            size_t first_atomic_size = find_first_atomic_member_size(member.base_type.getId());

            // Set the offset of this member within the struct according to the
            // standard alignment rules. The c++ standard specifies that:
            // > objects have an alignment requirement of which their size is a multiple
            member.offset = current_size + _H5_STRUCT_PADDING(current_size, first_atomic_size);

            // Set the current size to the end of the new member
            current_size = member.offset + member_size;

            // Keep track of the highest atomic member size because it's needed
            // for the padding of the complete compound type.
            max_atomic_size = std::max(max_atomic_size, first_atomic_size);
        }

        size = current_size + _H5_STRUCT_PADDING(current_size, max_atomic_size);
    }

    // Create the HDF5 type
    _hid = detail::h5t_create(H5T_COMPOUND, size);

    // Loop over all the members and insert them into the datatype
    for (const auto& member: members) {
        detail::h5t_insert(_hid, member.name.c_str(), member.offset, member.base_type.getId());
    }
}

#undef _H5_STRUCT_PADDING

inline void CompoundType::commit(const Object& object, const std::string& name) const {
    detail::h5t_commit2(
        object.getId(), name.c_str(), getId(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
}

template <typename T>
inline void EnumType<T>::create() {
    // Create the HDF5 type
    _hid = detail::h5t_enum_create(AtomicType<typename std::underlying_type<T>::type>{}.getId());

    // Loop over all the members and insert them into the datatype
    for (const auto& member: members) {
        detail::h5t_enum_insert(_hid, member.name.c_str(), &(member.value));
    }
}

template <typename T>
inline void EnumType<T>::commit(const Object& object, const std::string& name) const {
    detail::h5t_commit2(
        object.getId(), name.c_str(), getId(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
}

namespace {

inline hid_t create_string(size_t length) {
    hid_t _hid = detail::h5t_copy(H5T_C_S1);
    detail::h5t_set_size(_hid, length);
    detail::h5t_set_cset(_hid, H5T_CSET_UTF8);
    return _hid;
}


inline DataTypeClass convert_type_class(const H5T_class_t& tclass) {
    switch (tclass) {
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
    switch (tclass) {
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

}  // unnamed namespace


/// \brief Create a DataType instance representing type T
template <typename T>
inline DataType create_datatype() {
    return AtomicType<T>();
}


/// \brief Create a DataType instance representing type T and perform a sanity check on its size
template <typename T>
inline DataType create_and_check_datatype() {
    DataType t = create_datatype<T>();
    if (t.empty()) {
        throw DataTypeException("Type given to create_and_check_datatype is not valid");
    }

    // Skip check if the base type is a variable length string
    if (t.isVariableStr()) {
        return t;
    }

    // Check that the size of the template type matches the size that HDF5 is
    // expecting.
    if (t.isReference() || t.isFixedLenStr()) {
        return t;
    }
    if (sizeof(T) != t.getSize()) {
        std::ostringstream ss;
        ss << "Size of array type " << sizeof(T) << " != that of memory datatype " << t.getSize()
           << std::endl;
        throw DataTypeException(ss.str());
    }

    return t;
}

}  // namespace HighFive
HIGHFIVE_REGISTER_TYPE(HighFive::details::Boolean, HighFive::create_enum_boolean)

namespace HighFive {

template <>
inline DataType create_datatype<bool>() {
    return create_datatype<HighFive::details::Boolean>();
}

}  // namespace HighFive
