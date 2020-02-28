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
#include <cstring>

#include <H5Ppublic.h>
#include <H5Tpublic.h>


namespace HighFive {

namespace {  // unnamed
inline DataTypeClass convert_type_class(const H5T_class_t& tclass);
inline std::string type_class_string(DataTypeClass);
inline hid_t create_string(std::size_t length);
}


inline DataType::DataType() {}

inline DataType::DataType(hid_t type_hid) { // protected
    _hid = type_hid;
}

inline bool DataType::empty() const noexcept {
    return _hid == H5I_INVALID_HID;
}

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
    auto var_value = H5Tis_variable_str(_hid);
    if (var_value < 0) {
         HDF5ErrMapper::ToException<DataTypeException>(
            "Unable to define datatype size to variable");
    }
    return static_cast<bool>(var_value);
}

inline bool DataType::isFixedLenStr() const {
    return getClass() == DataTypeClass::String && !isVariableStr();
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
    _hid = create_string(H5T_VARIABLE);
}

// Fixed-Length strings
// require class specialization templated for the char length
template <size_t StrLen>
class AtomicType<char[StrLen]> : public DataType {
  public:
    inline AtomicType() {
        _hid = create_string(StrLen);
    }
};

template <size_t StrLen>
class AtomicType<FixedLenStringArray<StrLen>> : public DataType {
  public:
    inline AtomicType() {
        _hid = create_string(StrLen);
    }
};

template <>
inline AtomicType<std::complex<double> >::AtomicType() {
    static struct ComplexType : public Object {
        ComplexType() {
            _hid = H5Tcreate(H5T_COMPOUND, sizeof(std::complex<double>));
            // h5py/numpy compatible datatype
            H5Tinsert(_hid, "r", 0, H5T_NATIVE_DOUBLE);
            H5Tinsert(_hid, "i", sizeof(double), H5T_NATIVE_DOUBLE);
        };
    } complexType;
    _hid = H5Tcopy(complexType.getId());
}

// Other cases not supported. Fail early with a user message
template <typename T>
AtomicType<T>::AtomicType() {
    static_assert(details::array_dims<T>::value == 0,
                  "Atomic types cant be arrays, except for char[] (fixed-len strings)");
    static_assert(details::array_dims<T>::value > 0, "Type not supported");
}


// class FixedLenStringArray<N>

template <std::size_t N>
inline FixedLenStringArray<N>
::FixedLenStringArray(const char array[][N], std::size_t length) {
    datavec.resize(length);
    std::memcpy(datavec[0].data(), array[0].data(), N * length);
}

template <std::size_t N>
inline FixedLenStringArray<N>
::FixedLenStringArray(const std::string* iter_begin, const std::string* iter_end) {
    datavec.resize(static_cast<std::size_t>(iter_end - iter_begin));
    for (auto& dst_array : datavec) {
        const char* src = (iter_begin++)->c_str();
        const size_t length = std::min(N - 1 , std::strlen(src));
        std::memcpy(dst_array.data(), src, length);
        dst_array[length] = 0;
    }
}

template <std::size_t N>
inline FixedLenStringArray<N>
::FixedLenStringArray(const std::vector<std::string> & vec)
    : FixedLenStringArray(&vec.front(), &vec.back()) {}

template <std::size_t N>
inline FixedLenStringArray<N>
::FixedLenStringArray(const std::initializer_list<std::string>& init_list)
    : FixedLenStringArray(init_list.begin(), init_list.end()) {}

template <std::size_t N>
inline void FixedLenStringArray<N>::push_back(const std::string& src) {
    datavec.emplace_back();
    const size_t length = std::min(N - 1 , src.length());
    std::memcpy(datavec.back().data(), src.c_str(), length);
    datavec.back()[length] = 0;
}

template <std::size_t N>
inline void FixedLenStringArray<N>::push_back(const std::array<char, N>& src) {
    datavec.emplace_back();
    std::copy(src.begin(), src.end(), datavec.back().data());
}

template <std::size_t N>
inline std::string FixedLenStringArray<N>::getString(std::size_t i) const {
    return std::string(datavec[i].data());
}

// Internal

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

namespace {

inline hid_t create_string(size_t length){
    hid_t _hid = H5Tcopy(H5T_C_S1);
    if (H5Tset_size(_hid, length) < 0) {
        HDF5ErrMapper::ToException<DataTypeException>(
            "Unable to define datatype size to variable");
    }
    // define encoding to UTF-8 by default
    H5Tset_cset(_hid, H5T_CSET_UTF8);
    return _hid;
}


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
    // Skip check if the base type is a variable length string
    if (t.isVariableStr()) {
        return t;
    }
    // Check that the size of the template type matches the size that HDF5 is
    // expecting.
    if (sizeof(T) != t.getSize()) {
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
