/*
 * Copyright (C) 2016 Ali Can Demiralp <ali.demiralp@rwth-aachen.de>
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
#ifndef H5ATTRIBUTE_MISC_HPP
#define H5ATTRIBUTE_MISC_HPP



#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include <numeric>

#ifdef H5_USE_BOOST
#include <boost/multi_array.hpp>
#endif

#include <H5Apublic.h>
#include <H5Ppublic.h>

#include "../H5Attribute.hpp"
#include "../H5DataType.hpp"
#include "../H5DataSpace.hpp"

#include "H5Converter_misc.hpp"
#include "H5Utils.hpp"

namespace HighFive{

inline Attribute::Attribute() {}


inline size_t Attribute::getStorageSize() const{
    return H5Aget_storage_size(_hid);
}


inline DataType Attribute::getDataType() const{
    DataType res;
    res._hid = H5Aget_type(_hid);
    return res;
}


inline DataSpace Attribute::getSpace() const{
    DataSpace space;
    if( (space._hid = H5Aget_space(_hid)) < 0){
        HDF5ErrMapper::ToException<AttributeException>("Unable to get DataSpace out of Attribute");
    }
    return space;
}



inline DataSpace Attribute::getMemSpace() const{
    return getSpace();
}



template <typename T>
inline void Attribute::read(T & array) const {
    const size_t dim_array = details::array_dims<T>::value;
    DataSpace space     = getSpace   ();
    DataSpace mem_space = getMemSpace();

    const size_t dim_attribute = mem_space.getNumberDimensions();
    if (dim_array != dim_attribute) {
      std::ostringstream ss;
      ss << "Impossible to read attribute of dimensions " << dim_attribute << " into arrays of dimensions " << dim_array;
      throw DataSpaceException(ss.str());
    }

    // Create mem datatype
    const AtomicType<typename details::type_of_array<T>::type > array_datatype;

    // Apply pre read convertions
    details::data_converter<T> converter(array, mem_space);

    if (H5Aread(getId(), array_datatype.getId(), static_cast<void*>(converter.transform_read(array))) < 0) {
      HDF5ErrMapper::ToException<AttributeException>("Error during HDF5 Read: ");
    }

    // re-arrange results
    converter.process_result(array);
}


template <typename T>
inline void Attribute::write(T & buffer) {
    const size_t dim_buffer = details::array_dims<T>::value;
    DataSpace space     = getSpace   ();
    DataSpace mem_space = getMemSpace();

    const size_t dim_attribute = mem_space.getNumberDimensions();
    if (dim_buffer != dim_attribute) {
      std::ostringstream ss;
      ss << "Impossible to write buffer of dimensions " << dim_buffer << " into attribute of dimensions " << dim_attribute;
      throw DataSpaceException(ss.str());
    }

    const AtomicType<typename details::type_of_array<T>::type > array_datatype;

    // Apply pre write convertions
    details::data_converter<T> converter(buffer, mem_space);

    if (H5Awrite(getId(), array_datatype.getId(), static_cast<const void*>(converter.transform_write(buffer))) < 0) {
      HDF5ErrMapper::ToException<DataSetException>("Error during HDF5 Write: ");
    }
}


}

#endif // H5ATTRIBUTE_MISC_HPP
