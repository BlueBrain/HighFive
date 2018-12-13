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

namespace {
inline hid_t convert_plist_type(Properties::Type propertyType) {
    // The HP5_XXX are macros with function calls so we can't assign
    // them as the enum values
    switch(propertyType) {
    case Properties::OBJECT_CREATE:
        return H5P_OBJECT_CREATE;
    case Properties::FILE_CREATE:
        return H5P_FILE_CREATE;
    case Properties::FILE_ACCESS:
        return H5P_FILE_ACCESS;
    case Properties::DATASET_CREATE:
        return H5P_DATASET_CREATE;
    case Properties::DATASET_ACCESS:
        return H5P_DATASET_ACCESS;
    case Properties::DATASET_XFER:
        return H5P_DATASET_XFER;
    case Properties::GROUP_CREATE:
        return H5P_GROUP_CREATE;
    case Properties::GROUP_ACCESS:
        return H5P_GROUP_ACCESS;
    case Properties::DATATYPE_CREATE:
        return H5P_DATATYPE_CREATE;
    case Properties::DATATYPE_ACCESS:
        return H5P_DATATYPE_ACCESS;
    case Properties::STRING_CREATE:
        return H5P_STRING_CREATE;
    case Properties::ATTRIBUTE_CREATE:
        return H5P_ATTRIBUTE_CREATE;
    case Properties::OBJECT_COPY:
        return H5P_OBJECT_COPY;
    case Properties::LINK_CREATE:
        return H5P_LINK_CREATE;
    case Properties::LINK_ACCESS:
        return H5P_LINK_ACCESS;
    default:
            HDF5ErrMapper::ToException<PropertyException>(
                "Unsupported property list type");
    }
    return -1;
}

}  // namespace


inline Properties::Properties(Type type)
  : _type(type)
  , _hid(H5P_DEFAULT)
{}

#ifdef H5_USE_CXX11
inline Properties::Properties(Properties&& other)
    : _type(other._type)
    , _hid(other._hid)
{
    other._hid = H5P_DEFAULT;
}

inline Properties& Properties::operator=(Properties&& other)
{
    _type = other._type;
    // This code handles self-assigment without ifs
    const auto hid = other._hid;
    other._hid = H5P_DEFAULT;
    _hid = hid;
    return *this;
}
#endif

inline Properties::~Properties()
{
    // H5P_DEFAULT and H5I_INVALID_HID are not the same Ensuring that ~Object
    if (_hid != H5P_DEFAULT) {
        H5Pclose(_hid);
    }
}

inline void Properties::_initializeIfNeeded() {
    if (_hid != H5P_DEFAULT) { return; }
    if ((_hid = H5Pcreate(convert_plist_type(_type))) < 0) {
        HDF5ErrMapper::ToException<PropertyException>(
            "Unable to create property list");
    }
}

template <typename Property>
inline void Properties::add(const Property& property)
{
    _initializeIfNeeded();
    property.apply(_hid);
}


inline RawPropertyList::RawPropertyList(Type type)
  : Properties(type)
{}

template <typename T, typename... Args>
inline void RawPropertyList::add(const T& funct, const Args&... args)
{
    _initializeIfNeeded();
    if (funct(_hid, args...) < 0) {
        HDF5ErrMapper::ToException<PropertyException>(
            "Error setting raw hdf5 property.");
    }
}


inline void Chunking::apply(const hid_t hid) const
{
    if (H5Pset_chunk(hid, _dims.size(), _dims.data()) < 0)
    {
        HDF5ErrMapper::ToException<PropertyException>(
            "Error setting chunk property");
    }
}

inline void Deflate::apply(const hid_t hid) const
{
    if (!H5Zfilter_avail(H5Z_FILTER_DEFLATE))
    {
        HDF5ErrMapper::ToException<PropertyException>(
            "Error setting deflate property");
    }

    if (H5Pset_deflate(hid, _level) < 0)
    {
        HDF5ErrMapper::ToException<PropertyException>(
            "Error setting deflate property");
    }
}

inline void Shuffle::apply(const hid_t hid) const
{
    if (!H5Zfilter_avail(H5Z_FILTER_SHUFFLE))
    {
        HDF5ErrMapper::ToException<PropertyException>(
            "Error setting shuffle property");
    }

    if (H5Pset_shuffle(hid) < 0)
    {
        HDF5ErrMapper::ToException<PropertyException>(
            "Error setting shuffle property");
    }
}

inline void Caching::apply(const hid_t hid) const
{
    if (H5Pset_chunk_cache(hid, _numSlots, _cacheSize, _w0) < 0)
    {
        HDF5ErrMapper::ToException<PropertyException>(
            "Error setting dataset cache parameters");
    }
}
}
#endif // H5PROPERTY_LIST_HPP
