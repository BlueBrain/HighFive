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
#ifndef H5DATATYPE_HPP
#define H5DATATYPE_HPP

#include "H5Object.hpp"



namespace HighFive{

struct TypeMapper;

///
/// \brief HDF5 Data Type
///
class DataType : public Object{
public:
    DataType();

    bool operator==(const DataType & other) const;

    bool operator!=(const DataType & other) const;

protected:

    friend class Attribute;
    friend class File;
    friend class DataSet;
};


///
/// \brief create an HDF5 DataType from a C++ type
///
///  Support only basic data type
///
template<typename T>
class AtomicType : public DataType{
public:

    AtomicType();


    typedef T basic_type;
};


}

#include "bits/H5DataType_misc.hpp"

#endif // H5DATATYPE_HPP
