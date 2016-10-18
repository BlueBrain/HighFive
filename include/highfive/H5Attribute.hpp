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
#ifndef H5ATTRIBUTE_HPP
#define H5ATTRIBUTE_HPP

#include <vector>

#include "H5Object.hpp"




namespace HighFive{


template <typename Derivate> class NodeTraits;
class DataType;
class DataSpace;

class Attribute : public Object{
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



    ///
    /// Read the attribute into a buffer
    /// An exception is raised if the numbers of dimension of the buffer and of the attribute are different
    ///
    /// The array type can be a N-pointer or a N-vector ( e.g int** integer two dimensional array )
    template <typename T>
    void read(T & array) const;

    ///
    /// Write the integrality N-dimension buffer to this attribute
    /// An exception is raised if the numbers of dimension of the buffer and of the attribute are different
    ///
    /// The array type can be a N-pointer or a N-vector ( e.g int** integer two dimensional array )
    template <typename T>
    void write(T & buffer);



private:
    Attribute();
    template <typename Derivate>
    friend class ::HighFive::NodeTraits;

};

}

#include "bits/H5Attribute_misc.hpp"

#endif // H5ATTRIBUTE_HPP
