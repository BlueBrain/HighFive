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
#ifndef H5DATASET_HPP
#define H5DATASET_HPP

#include <vector>

#include "H5Object.hpp"
#include "bits/H5Slice_traits.hpp"




namespace HighFive{


template <typename Derivate> class NodeTraits;
template <typename Derivate> class SliceTraits;
class DataType;
class DataSpace;

class DataSet : public SliceTraits<DataSet>, public Object{
public:

    size_t getStorageSize() const;

    ///
    /// \brief getDataType
    /// \return return the datatype associated with this dataset
    ///
    DataType getDataType() const;

    ///
    /// \brief getSpace
    /// \return return the dataspace associated with this dataset
    ///
    DataSpace getSpace() const;


    ///
    /// \brief getMemSpace
    /// \return same than getSpace for DataSet, compatibility with Selection class
    ///
    DataSpace getMemSpace() const;

private:
    DataSet();
    template <typename Derivate>
    friend class ::HighFive::NodeTraits;

};

}

#include "bits/H5DataSet_misc.hpp"

#endif // H5DATASET_HPP
