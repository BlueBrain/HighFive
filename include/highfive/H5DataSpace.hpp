/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5DATASPACE_HPP
#define H5DATASPACE_HPP

#include <vector>
#include <array>
#include <cstdint>
#include <type_traits>
#include <initializer_list>

#ifdef H5_USE_BOOST

// In some versions of Boost (starting with 1.64), you have to include the serialization header before ublas
#include <boost/serialization/vector.hpp>

#include <boost/multi_array.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#endif

#include "H5Object.hpp"

namespace HighFive {

class File;
class DataSet;

class DataSpace : public Object {
  public:

    static const size_t UNLIMITED = SIZE_MAX;

    /// dataspace type
    enum DataspaceType {
        datascape_scalar,
        datascape_null
        // simple dataspace are handle directly from their dimensions
    };

    /// create a dataspace of N-dimensions
    /// Each dimension is configured this way
    ///  size(dim1) = vec[0]
    ///  size(dim2) = vec[1]
    ///  etc...
    explicit DataSpace(const std::vector<size_t>& dims);

    /// Make sure that DataSpace({1,2,3}) works on GCC. This is
    /// the shortcut form of the vector initalizer, but one some compilers (gcc)
    /// this does not resolve correctly without this constructor.
    explicit DataSpace(std::initializer_list<size_t> items);

    /// Allow directly listing 1 or more dimensions to initialize,
    /// that is, DataSpace(1,2) means DataSpace(std::vector<size_t>{1,2}).
    template<typename... Args>
    explicit DataSpace(size_t dim1, Args... dims);

    /// Create a dataspace from an iterator pair
    ///
    /// Explicitly disable DataSpace(int_like, int_like) from trying to use this constructor
    template <typename IT, typename = typename std::enable_if<!std::is_integral<IT>::value,IT>::type>
    DataSpace(const IT begin,
              const IT end);

    /// \brief Create a resizable N-dimensional dataspace
    /// \params dims Initial size of dataspace
    /// \params maxdims Maximum size of the dataspace
    explicit DataSpace(const std::vector<size_t>& dims,
                       const std::vector<size_t>& maxdims);

    ///
    /// \brief DataSpace create a scalar dataspace or a null dataset
    ///
    explicit DataSpace(DataspaceType dtype);

    /// Create a new DataSpace
    ///  with a different id available for modifications
    DataSpace clone() const;

    ///
    /// \brief getNumberDimensions
    /// \return the number of dimensions in the current dataspace
    ///
    size_t getNumberDimensions() const;

    /// \brief getDimensions
    /// \return return a vector of N-element, each element is the size of the
    /// associated dataset dimension
    std::vector<size_t> getDimensions() const;

    /// \brief getElementCount
    /// \return the total number of elements in the dataspace
    size_t getElementCount() const;

    /// \brief getMaxDimensions
    /// \return return a vector of N-element, each element is the size of the
    /// associated dataset maximum dimension
    std::vector<size_t> getMaxDimensions() const;

    /// Create a dataspace matching a single element of a basic type
    ///  supported type are integrals (int,long), floating points (float,double)
    ///  and std::string
    template <typename ScalarValue>
    static DataSpace From(const ScalarValue& scalar_value);

    /// Create a dataspace matching the container dimensions and size
    /// Supported Containers are:
    ///  - vector of fundamental types
    ///  - vector of std::string
    ///  - boost::multi_array (with H5_USE_BOOST defined)
    template <typename Value>
    static DataSpace From(const std::vector<Value>& vec);

    /// Create a dataspace matching the container dimensions for a
    /// std::array.
    template <typename Value, std::size_t N>
    static DataSpace From(const std::array<Value, N>&);


#ifdef H5_USE_BOOST
    template <typename Value, std::size_t Dims>
    static DataSpace From(const boost::multi_array<Value, Dims>& container);

    template <typename Value>
    static DataSpace From(const boost::numeric::ublas::matrix<Value>& mat);
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
