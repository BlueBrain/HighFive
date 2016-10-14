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
#ifndef H5DATASPACE_HPP
#define H5DATASPACE_HPP

#include <vector>
#ifdef H5_USE_BOOST
#include <boost/multi_array.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#endif

#include "H5Object.hpp"



namespace HighFive{

class File;
class DataSet;

class DataSpace : public Object{
public:
    /// create a dataspace of N-dimensions
    /// Each dimension is configured this way
    ///  size(dim1) = vec[0]
    ///  size(dim2) = vec[1]
    ///  etc...
    explicit DataSpace(const std::vector<size_t> & dims);

    ///
    /// \brief DataSpace create a dataspace of a single dimension and of size dim1
    /// \param dim1
    ///
    explicit DataSpace(size_t dim1);

    /// Create a new DataSpace
    ///  with a different id avaiable for modifications
    DataSpace clone() const;

    ///
    /// \brief getNumberDimensions
    /// \return the number of dimensions in the current dataspace
    ///
    size_t getNumberDimensions() const;

    /// \brief getDimensions
    /// \return return a vector of N-element, each element is the size of the associated dataset dimension
    std::vector<size_t> getDimensions() const;


    /// Create a dataspace matching the container dimensions and size
    /// Supported Containers are:
    ///  - vector of fundamental types
    ///  - vector of std::string
    ///  - boost::multi_array
    template<typename Value>
    static DataSpace From(const std::vector<Value> & vec);

#ifdef H5_USE_BOOST
    template<typename Value, std::size_t Dims>
    static DataSpace From(const boost::multi_array<Value, Dims> & container);

    template<typename Value>
    static DataSpace From(const boost::numeric::ublas::matrix<Value> & mat);
#endif



protected:

    explicit DataSpace();

    friend class Attribute;
    friend class File;
    friend class DataSet;
};

}

#include "bits/H5Dataspace_misc.hpp"

#endif // H5DATASPACE_HPP
