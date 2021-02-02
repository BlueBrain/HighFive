/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5OBJECT_MISC_HPP
#define H5OBJECT_MISC_HPP

#include <iostream>
#include <H5Fpublic.h>

namespace HighFive {

ObjectType _convert_object_type(const H5I_type_t& h5type);

inline Object::Object() : _hid(H5I_INVALID_HID) {}

inline Object::Object(const hid_t& hid, const ObjectType& objType) {
    if (hid < 0) {
        HDF5ErrMapper::ToException<FileException>(
            std::string("Invalid id to initialize the object"));
        return;
    }

    ObjectType objType_from_id = _convert_object_type(H5Iget_type(hid));
    if (objType_from_id != objType){
        HDF5ErrMapper::ToException<FileException>(
            std::string("Given id doesn't belong to the requested type"));
        return;
    }

    _hid = hid;
}

inline Object::Object(const Object& other) : _hid(other._hid) {
    if (other.isValid() && H5Iinc_ref(_hid) < 0) {
        throw ObjectException("Reference counter increase failure");
    }
}

inline Object::Object(Object&& other) noexcept
    : _hid(other._hid)  {
    other._hid = H5I_INVALID_HID;
}

inline Object& Object::operator=(const Object& other) {
    if (this != &other) {
        if (_hid != H5I_INVALID_HID)
            H5Idec_ref(_hid);

        _hid = other._hid;
        if (other.isValid() && H5Iinc_ref(_hid) < 0) {
            throw ObjectException("Reference counter increase failure");
        }
    }
    return *this;
}

inline Object::~Object() {
    if (isValid() && H5Idec_ref(_hid) < 0) {
        std::cerr << "HighFive::~Object: reference counter decrease failure"
                  << std::endl;
    }
}

inline bool Object::isValid() const noexcept {
    return (_hid != H5I_INVALID_HID) && (H5Iis_valid(_hid) != false);
}

inline hid_t Object::getId() const noexcept {
    return _hid;
}

inline hid_t Object::getFileId() const noexcept {
    return H5Iget_file_id(_hid);
}

inline bool Object::operator==(const Object& other) const {

//    if (getInfo().getAddress() != other.getInfo().getAddress())
//        return false;

    /* I would better implement this block to check file equality
     * but `H5Fget_fileno` was introduced only since hdf5 1.12.0 */

//    unsigned long num, num_other;
//    herr_t err = H5Fget_fileno(getFileId(), &num);
//    if (err < 0)
//        return false;

//    err = H5Fget_fileno(other.getId(), &num_other);
//    if (err < 0)
//        return false;

//    return num == num_other;

    /* But for now I have to compare filenames */

    char name[256];
    ssize_t st = H5Fget_name(getFileId(), name, 256);
    if (st < 0)
        return false;
    std::string str_name = std::string{name};

    st = H5Fget_name(other.getFileId(), name, 256);
    if (st < 0)
        return false;
    std::string str_name_other = std::string{name};

    int val = str_name.compare(str_name_other);
    if (val != 0)
        return false;
    
    return true;
}

inline bool Object::operator!=(const Object& other) const {
    return !(*this == other);
}

static inline ObjectType _convert_object_type(const H5I_type_t& h5type) {
    switch (h5type) {
        case H5I_FILE:
            return ObjectType::File;
        case H5I_GROUP:
            return ObjectType::Group;
        case H5I_DATATYPE:
            return ObjectType::UserDataType;
        case H5I_DATASPACE:
            return ObjectType::DataSpace;
        case H5I_DATASET:
            return ObjectType::Dataset;
        case H5I_ATTR:
            return ObjectType::Attribute;
        default:
            return ObjectType::Other;
    }
}

static inline H5I_type_t _convert_object_type_back(const ObjectType& type) {
    switch (type) {
        case ObjectType::File:
            return H5I_FILE;
        case ObjectType::Group:
            return H5I_GROUP;
        case ObjectType::UserDataType:
            return H5I_DATATYPE;
        case ObjectType::DataSpace:
            return H5I_DATASPACE;
        case ObjectType::Dataset:
            return H5I_DATASET;
        case ObjectType::Attribute:
            return H5I_ATTR;
        default:
            return H5I_BADID;
    }
}

inline ObjectType Object::getType() const {
    // H5Iget_type is a very lightweight func which extracts the type from the id
    H5I_type_t h5type;
    if ((h5type = H5Iget_type(_hid)) == H5I_BADID) {
        HDF5ErrMapper::ToException<ObjectException>("Invalid hid or object type");
    }
    return _convert_object_type(h5type);
}

inline ObjectInfo Object::getInfo() const {
    ObjectInfo info;
#if (H5Oget_info_vers < 3)
    if (H5Oget_info(_hid, &info.raw_info) < 0) {
#else
    if (H5Oget_info1(_hid, &info.raw_info) < 0) {
#endif
        HDF5ErrMapper::ToException<ObjectException>("Unable to obtain info for object");
    }
    return info;
}

inline haddr_t ObjectInfo::getAddress() const noexcept {
    return raw_info.addr;
}
inline size_t ObjectInfo::getRefCount() const noexcept {
    return raw_info.rc;
}

inline time_t ObjectInfo::getCreationTime() const noexcept {
    return raw_info.btime;
}
inline time_t ObjectInfo::getModificationTime() const noexcept {
    return raw_info.mtime;
}



}  // namespace

#endif // H5OBJECT_MISC_HPP
