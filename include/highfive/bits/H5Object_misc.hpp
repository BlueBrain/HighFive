/*
 * Copyright (C) 2015 Adrien Devresse <adrien.devresse@epfl.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
