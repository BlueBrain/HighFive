/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <type_traits>
#include <initializer_list>

#include "H5Object.hpp"
#include "bits/H5_definitions.hpp"

namespace HighFive {

/// \brief Class representing the space (dimensions) of a DataSet
///
/// \code{.cpp}
/// // Create a DataSpace of dimension 1 x 2 x 3
/// DataSpace dspace(1, 2, 3);
/// std::cout << dspace.getElementCount() << std::endl; // Print 1 x 2 x 3 => 6
/// std::cout << dspace.getNumberDimensions() << std::endl; // Print 3
/// std::vector<size_t> dims = dspace.getDimensions(); // dims is {1, 2, 3}
/// \endcode
class DataSpace: public Object {
  public:
    const static ObjectType type = ObjectType::DataSpace;

    /// \brief To specify that a DataSpace can grow without limit.
    ///
    /// This value should be used with DataSpace::DataSpace(const std::vector<size_t>& dims, const
    /// std::vector<size_t>& maxdims); \since 2.0
    static const size_t UNLIMITED = SIZE_MAX;

    /// \brief An enum to create scalar and null DataSpace with DataSpace::DataSpace(DataspaceType dtype).
    ///
    /// This enum is needed otherwise we will not be able to distringuish between both with normal
    /// constructors. Both have a dimension of 0.
    /// \since 1.3
    enum DataspaceType {
        dataspace_scalar,  ///< Value to create scalar DataSpace
        dataspace_null,    ///< Value to create null DataSpace
        // simple dataspace are handle directly from their dimensions
    };

    /// \brief Create a DataSpace of N-dimensions from a std::vector<size_t>.
    /// \param dims Dimensions of the new DataSpace
    /// \since 1.0
    /// \code{.cpp}
    /// // Create a DataSpace with 2 dimensions: 1 and 3
    /// DataSpace(std::vector<size_t>({1, 3}));
    /// \endcode
    explicit DataSpace(const std::vector<size_t>& dims);

    /// \brief Create a DataSpace of N-dimensions from a std::array<size_t, N>.
    /// \param dims Dimensions of the new DataSpace
    /// \since 2.3
    /// \code{.cpp}
    /// // Create a DataSpace with 2 dimensions: 1 and 3
    /// DataSpace(std::array<size_t, 2>(1, 3));
    /// \endcode
    template <size_t N>
    explicit DataSpace(const std::array<size_t, N>& dims);

    /// \brief Create a DataSpace of N-dimensions from an initializer list.
    /// \param dims Dimensions of the new DataSpace
    /// \since 2.1
    /// \code{.cpp}
    /// // Create a DataSpace with 2 dimensions: 1 and 3
    /// DataSpace({1, 3});
    /// \endcode
    DataSpace(const std::initializer_list<size_t>& dims);

    /// \brief Create a DataSpace of N-dimensions from direct values.
    /// \param dim1 The first dimension
    /// \param dims The following dimensions
    /// \since 2.1
    /// \code{.cpp}
    /// // Create a DataSpace with 2 dimensions: 1 and 3
    /// DataSpace(1, 3);
    /// \endcode
    template <typename... Args>
    explicit DataSpace(size_t dim1, Args... dims);

    /// \brief Create a DataSpace from an iterator pair.
    /// \param begin The beginning of the container
    /// \param end The end of the container
    /// \since 2.0
    /// \code{.cpp}
    /// // Create a DataSpace with 2 dimensions: 1 and 3
    /// std::vector<int> v({1, 3});
    /// DataSpace(v.begin(), v.end());
    /// \endcode
    ///
    /// \attention Explicitly disable DataSpace(int_like, int_like) from trying to use this constructor
    template <typename IT,
              typename = typename std::enable_if<!std::is_integral<IT>::value, IT>::type>
    DataSpace(const IT begin, const IT end);

    /// \brief Create a resizable N-dimensional DataSpace.
    /// \param dims Initial size of dataspace
    /// \param maxdims Maximum size of the dataspace
    /// \since 2.0
    /// \code{.cpp}
    /// // Create a DataSpace with 2 dimensions: 1 and 3.
    /// // It can be resize later until 10, 10
    /// DataSpace(std::vector<size_t>({1, 3}), std::vector<size_t>({10, 10});
    /// \endcode
    ///
    /// \see UNLIMITED for a DataSpace that can be resize without limit.
    explicit DataSpace(const std::vector<size_t>& dims, const std::vector<size_t>& maxdims);

    /// \brief Create a scalar or a null DataSpace.
    ///
    /// This overload exists because otherwise we will not be able to distinguish between
    /// scalar and null DataSpace that got both 0 dimension.
    /// \param dtype The value from the enum
    /// \since 1.3
    /// \code{.cpp}
    /// DataSpace(DataspaceType::dataspace_scalar);
    /// \endcode
    explicit DataSpace(DataspaceType dtype);

    /// \brief Create a new DataSpace with a different id available for modifications.
    /// \since 1.0
    /// \code{.cpp}
    /// DataSpace dspace1(1, 3);
    /// auto dspace2 = dspace.clone();
    /// \endcode
    DataSpace clone() const;

    /// \brief Give the number of dimensions of a DataSpace.
    /// \return the number of dimensions in the current DataSpace
    /// \since 1.0
    /// \code{.cpp}
    /// DataSpace dspace(1, 3);
    /// size_t number_of_dim = dspace.getNumberDimensions(); // return 2
    /// \endcode
    size_t getNumberDimensions() const;

    /// \brief Return a vector of all dimensions.
    /// \return return a vector of N-element, each element is the size of the
    /// associated dataset dimension
    /// \since 1.0
    /// \code{.cpp}
    /// DataSpace dspace(1, 3);
    /// auto dims = dspace.getDimensions(); // return {1, 3}
    /// \endcode
    std::vector<size_t> getDimensions() const;

    /// \brief Return the number of elements in this DataSpace.
    /// \return the total number of elements in the DataSpace
    /// \since 2.1
    /// \code{.cpp}
    /// DataSpace dspace(1, 3);
    /// size_t elementcount = dspace.getElementCount(); // return 1 x 3 = 3
    /// \endcode
    size_t getElementCount() const;

    /// \brief Return the maximum dimensions of this DataSpace.
    /// \return return a vector of N-element, each element is the size of the
    /// associated dataset maximum dimension
    /// \since 2.0
    /// \code{.cpp}
    /// DataSpace dspace(std::vector<size_t>({1, 3}), std::vector<size_t>({UNLIMITED, 10});
    /// dspace.getMaxDimensions(); // Return {UNLIMITED, 10}
    /// \endcode
    std::vector<size_t> getMaxDimensions() const;

    /// \brief Create a DataSpace from a internally deduced value.
    /// \param value The multi-dimensional to fit in the DataSpace.
    /// \return A new DataSpace.
    /// \since 1.0
    /// \code{.cpp}
    /// std::vector<std::vector<int>> v = {{4, 5, 6}, {7, 8, 9}};
    /// auto dspace = DataSpace::From(v); // dspace is a DataSpace of dimensions 2, 3
    /// \endcode
    template <typename T>
    static DataSpace From(const T& value);

    /// \brief Create a DataSpace from a value of type string array.
    /// \param string_array An C-array of C-string.
    /// \return A new DataSpace.
    /// \since 2.2
    /// \code{.cpp}
    /// char string_array[2][10] = {"123456789", "abcdefghi"};
    /// auto dspace = DataSpace::FromCharArrayStrings(string_array); // dspace is a DataSpace of
    /// dimensions 2 \endcode
    template <std::size_t N, std::size_t Width>
    static DataSpace FromCharArrayStrings(const char (&string_array)[N][Width]);

  protected:
    DataSpace() = default;

    friend class Attribute;
    friend class File;
    friend class DataSet;
};

}  // namespace HighFive

// We include bits right away since DataSpace is user-constructible
#include "bits/H5Dataspace_misc.hpp"
