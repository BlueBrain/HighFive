/*
 *  Copyright (c), 2017-2018, Adrien Devresse <adrien.devresse@epfl.ch>
 *                            Juan Hernando <juan.hernando@epfl.ch>
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5PROPERTY_LIST_MISC_HPP
#define H5PROPERTY_LIST_MISC_HPP

#include <H5Ppublic.h>

#include "../H5PropertyList.hpp"

namespace HighFive {

inline Properties::Properties(Type type)
  : _type(type)
  , _hid(H5P_DEFAULT)
{
}

inline Properties::~Properties()
{
    // H5P_DEFAULT and H5I_INVALID_HID are not the same Ensuring that ~Object
    if (_hid != H5P_DEFAULT)
        H5Pclose(_hid);
}

template <typename Property>
inline void Properties::add(const Property& property)
{
    if (_hid == H5P_DEFAULT)
    {
        hid_t type;
        // The HP5_XXX are macros with function calls
        switch (_type)
        {
        case FILE_ACCESS: {
            type = H5P_FILE_ACCESS;
            break;
        }
        case DATASET_CREATE: {
            type = H5P_DATASET_CREATE;
            break;
        }
        default:
            HDF5ErrMapper::ToException<PropertyException>(
                std::string("Unsupported property list type"));
        }
        if ((_hid = H5Pcreate(type)) < 0) {
            HDF5ErrMapper::ToException<PropertyException>(
                std::string("Unable to create property list"));
        }
    }

    property.apply(_hid);
}
}
#endif // H5PROPERTY_LIST_HPP
