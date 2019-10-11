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

#include "../H5Exception.hpp"
#include "../H5Object.hpp"

#include <iostream>

namespace HighFive {

inline Object::Object() : _hid(H5I_INVALID_HID) {}

inline Object::Object(hid_t hid) : _hid(hid) {}

inline Object::Object(const Object& other) : _hid(other._hid) {
    if (other.isValid() && H5Iinc_ref(_hid) < 0) {
        throw ObjectException("Reference counter increase failure");
    }
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

inline bool Object::isValid() const {
    return (_hid != H5I_INVALID_HID) && (H5Iis_valid(_hid) != false);
}

inline hid_t Object::getId() const { return _hid; }

static ObjectType _convert_object_type(const H5I_type_t& h5type) {
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
            return ObjectType::Invalid;
    }
}

inline ObjectType Object::getType() const {
    return _convert_object_type(H5Iget_type(_hid));
}


ObjectInfo Object::getInfo() const {
    ObjectInfo info;
    if (H5Oget_info(_hid, &info.raw_info) < 0) {
        HDF5ErrMapper::ToException<ObjectException>("Unable to obtain info for object");
    }
    return info;
}

haddr_t ObjectInfo::getAddress() const noexcept {
    return raw_info.addr;
}
size_t ObjectInfo::referenceCount() const noexcept {
    return raw_info.rc;
}
time_t ObjectInfo::creationTime() const noexcept {
    return raw_info.btime;
}
time_t ObjectInfo::modificationTime() const noexcept {
    return raw_info.mtime;
}



}  // namespace

#endif // H5OBJECT_MISC_HPP
