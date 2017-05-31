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

#include "../H5Object.hpp"
#include "../H5Exception.hpp"

namespace HighFive{

inline Object::Object() : _hid(H5I_INVALID_HID) {}

inline Object::Object(const Object &other) : _hid(other._hid){
    if(other.isValid() && H5Iinc_ref(_hid) <0){
            throw ObjectException("Reference counter increase failure");
    }
}

inline Object & Object::operator=(const Object &other) {
    if (this != &other) {
        _hid = other._hid;
        if(other.isValid() && H5Iinc_ref(_hid) <0){
            throw ObjectException("Reference counter increase failure");
        }
    }
    return *this;
}


inline Object::~Object(){
    destroy();
}

inline void Object::destroy(){
    if(isValid()  && H5Idec_ref(_hid) <0){
        throw ObjectException("Reference counter decrease failure");
    }
}


inline bool Object::isValid() const{
    return (_hid != H5I_INVALID_HID) && (H5Iis_valid(_hid) != false);
}


inline hid_t Object::getId() const{
    return _hid;
}



}

#endif // H5OBJECT_MISC_HPP
