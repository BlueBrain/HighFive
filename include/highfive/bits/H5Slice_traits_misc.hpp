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
#ifndef H5SLICE_TRAITS_MISC_HPP
#define H5SLICE_TRAITS_MISC_HPP

#include "H5Slice_traits.hpp"

#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include <numeric>
#include <cassert>

#ifdef H5_USE_BOOST
#include <boost/multi_array.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#endif

#include <H5Dpublic.h>
#include <H5Ppublic.h>

#include "../H5DataType.hpp"
#include "../H5DataSpace.hpp"
#include "../H5Selection.hpp"

#include "H5Converter_misc.hpp"
#include "H5Utils.hpp"

namespace HighFive{


namespace details{

// map the correct reference to the dataset depending of the layout
// dataset -> itself
// subselection -> parent dataset
inline const DataSet & get_dataset(const Selection* ptr){
    return ptr->getDataset();
}



inline const DataSet & get_dataset(const DataSet* ptr){
    return *ptr;
}


// map the correct memspace identifier depending of the layout
// dataset -> entire memspace
// selection -> resolve space id
inline hid_t get_memspace_id(const Selection* ptr){
    return ptr->getMemSpace().getId();
}


inline hid_t get_memspace_id(const DataSet* ptr){
    (void) ptr;
    return H5S_ALL;
}


}



template <typename Derivate>
inline Selection SliceTraits<Derivate>::select(const std::vector<size_t> & offset, const std::vector<size_t> & count) const{
    // hsize_t type convertion
    // TODO : normalize hsize_t type in HighFive namespace
    std::vector<hsize_t> offset_local(offset.size()), count_local(count.size());
    std::copy(offset.begin(), offset.end(), offset_local.begin());
    std::copy(count.begin(), count.end(), count_local.begin());

    DataSpace space = static_cast<const Derivate*>(this)->getSpace().clone();
    if( H5Sselect_hyperslab(space.getId(), H5S_SELECT_SET,  &(offset_local[0]), NULL, &(count_local[0]), NULL) < 0){
         HDF5ErrMapper::ToException<DataSpaceException>("Unable to select hyperslap");
    }

    return Selection(DataSpace(count), space, details::get_dataset(static_cast<const Derivate*>(this)));
}



template <typename Derivate>
template <typename T>
inline void SliceTraits<Derivate>::read(T & array) const{
    const size_t dim_array = details::array_dims<T>::value;
    DataSpace space = static_cast<const Derivate*>(this)->getSpace();
    DataSpace mem_space = static_cast<const Derivate*>(this)->getMemSpace();

    const size_t dim_dataset = mem_space.getNumberDimensions();
    if(dim_array != dim_dataset){
        std::ostringstream ss;
        ss << "Impossible to read DataSet of dimensions " << dim_dataset << " into arrays of dimensions " << dim_array;
        throw DataSpaceException(ss.str());
    }

    // Create mem datatype
    const AtomicType<typename details::type_of_array<T>::type > array_datatype;

    // Apply pre read convertions
    details::data_converter<T> converter(array, mem_space);

    if( H5Dread(details::get_dataset(static_cast<const Derivate*>(this)).getId(),
                array_datatype.getId(),
                details::get_memspace_id((static_cast<const Derivate*>(this))),
                space.getId(),
                H5P_DEFAULT,
                static_cast<void*>(converter.transform_read(array))) < 0){
         HDF5ErrMapper::ToException<DataSetException>("Error during HDF5 Read: ");
    }

    // re-arrange results
    converter.process_result(array);
}


template <typename Derivate>
template <typename T>
inline void SliceTraits<Derivate>::write(T & buffer){
    const size_t dim_buffer = details::array_dims<T>::value;
    DataSpace space = static_cast<const Derivate*>(this)->getSpace();
    DataSpace mem_space = static_cast<const Derivate*>(this)->getMemSpace();

    const size_t dim_dataset = mem_space.getNumberDimensions();
    if(dim_buffer != dim_dataset){
        std::ostringstream ss;
        ss << "Impossible to write buffer of dimensions " << dim_buffer << " into dataset of dimensions " << dim_dataset;
        throw DataSpaceException(ss.str());
    }

    const AtomicType<typename details::type_of_array<T>::type > array_datatype;

    // Apply pre write convertions
    details::data_converter<T> converter(buffer, mem_space);

    if( H5Dwrite(details::get_dataset(static_cast<Derivate*>(this)).getId(),
            array_datatype.getId(),
            details::get_memspace_id((static_cast<Derivate*>(this))),
            space.getId(),
            H5P_DEFAULT,
            static_cast<const void*>(converter.transform_write(buffer))) < 0){
         HDF5ErrMapper::ToException<DataSetException>("Error during HDF5 Write: ");
    }
}


}

#endif // H5SLICE_TRAITS_MISC_HPP
