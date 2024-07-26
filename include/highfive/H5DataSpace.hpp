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

namespace detail {
/// @brief Create a HighFive::DataSpace from an HID, without incrementing the id.
///
/// @note This is internal API and subject to change.
/// @internal
DataSpace make_data_space(hid_t hid);
}  // namespace detail

/// \brief Class representing the space (dimensions) of a DataSet
///
/// \code{.cpp}
/// // Create a DataSpace of dimension 1 x 2 x 3
/// DataSpace dspace(1, 2, 3);
/// std::cout << dspace.getElementCount() << std::endl; // Print 1 * 2 * 3 = 6
/// std::cout << dspace.getNumberDimensions() << std::endl; // Print 3
/// std::vector<size_t> dims = dspace.getDimensions(); // dims is {1, 2, 3}
/// \endcode
class DataSpace: public Object {
  public:
    const static ObjectType type = ObjectType::DataSpace;

    /// \brief Magic value to specify that a DataSpace can grow without limit.
    ///
    /// This value should be used with DataSpace::DataSpace(const std::vector<size_t>& dims, const
    /// std::vector<size_t>& maxdims);
    ///
    /// \since 2.0
    static const size_t UNLIMITED = SIZE_MAX;

    /// \brief An enum to create scalar and null DataSpace with DataSpace::DataSpace(DataspaceType dtype).
    ///
    /// This enum is needed otherwise we will not be able to distringuish between both with normal
    /// constructors. Both have a dimension of 0.
    /// \since 1.3
    enum class DataspaceType {
        dataspace_scalar,  ///< Value to create scalar DataSpace
        dataspace_null,    ///< Value to create null DataSpace
        // simple dataspace are handle directly from their dimensions
    };

    // For backward compatibility: `DataSpace::dataspace_scalar`.
    constexpr static DataspaceType dataspace_scalar = DataspaceType::dataspace_scalar;
    constexpr static DataspaceType dataspace_null = DataspaceType::dataspace_null;

    /// \brief Create a DataSpace of N-dimensions from a std::vector<size_t>.
    /// \param dims Dimensions of the new DataSpace
    ///
    /// \code{.cpp}
    /// // Create a DataSpace with 2 dimensions: 1 and 3
    /// DataSpace(std::vector<size_t>{1, 3});
    /// \endcode
    /// \since 1.0
    explicit DataSpace(const std::vector<size_t>& dims);

    /// \brief Create a DataSpace of N-dimensions from a std::array<size_t, N>.
    /// \param dims Dimensions of the new DataSpace
    ///
    /// \code{.cpp}
    /// // Create a DataSpace with 2 dimensions: 1 and 3
    /// DataSpace(std::array<size_t, 2>{1, 3});
    /// \endcode
    /// \since 2.3
    template <size_t N>
    explicit DataSpace(const std::array<size_t, N>& dims);

    /// \brief Create a DataSpace of N-dimensions from an initializer list.
    /// \param dims Dimensions of the new DataSpace
    ///
    /// \code{.cpp}
    /// // Create a DataSpace with 2 dimensions: 1 and 3
    /// DataSpace{1, 3};
    /// \endcode
    /// \since 2.1
    DataSpace(const std::initializer_list<size_t>& dims);

    /// \brief Create a DataSpace of N-dimensions from direct values.
    /// \param dim1 The first dimension
    /// \param dims The following dimensions
    ///
    /// \code{.cpp}
    /// // Create a DataSpace with 2 dimensions: 1 and 3
    /// DataSpace(1, 3);
    /// \endcode
    /// \since 2.1
    template <typename... Args>
    explicit DataSpace(size_t dim1, Args... dims);

    /// \brief Create a DataSpace from a pair of iterators.
    /// \param begin The beginning of the container
    /// \param end The end of the container
    ///
    /// \code{.cpp}
    /// // Create a DataSpace with 2 dimensions: 1 and 3
    /// std::vector<int> v{1, 3};
    /// DataSpace(v.begin(), v.end());
    /// \endcode
    ///
    /// \since 2.0
    // Attention: Explicitly disable DataSpace(int_like, int_like) from trying
    //            to use this constructor
    template <typename IT,
              typename = typename std::enable_if<!std::is_integral<IT>::value, IT>::type>
    DataSpace(const IT begin, const IT end);

    /// \brief Create a resizable N-dimensional DataSpace.
    /// \param dims Initial size of dataspace
    /// \param maxdims Maximum size of the dataspace
    ///
    /// \code{.cpp}
    /// // Create a DataSpace with 2 dimensions: 1 and 3.
    /// // It can later be resized up to a maximum of 10 x 10
    /// DataSpace(std::vector<size_t>{1, 3}, std::vector<size_t>{10, 10});
    /// \endcode
    ///
    /// \see UNLIMITED for a DataSpace that can be resized without limit.
    /// \since 2.0
    explicit DataSpace(const std::vector<size_t>& dims, const std::vector<size_t>& maxdims);

    /// \brief Create a scalar or a null DataSpace.
    ///
    /// This overload enables creating scalar or null data spaces, both have
    /// dimension 0.
    ///
    /// \param space_type The value from the enum
    ///
    /// \code{.cpp}
    /// DataSpace(DataspaceType::dataspace_scalar);
    /// \endcode
    ///
    /// \attention Avoid braced intialization in these cases, i.e.
    /// \code{.cpp}
    /// // This is not a scalar dataset:
    /// DataSpace{DataspaceType::dataspace_scalar};
    /// \endcode
    ///
    /// \since 1.3
    explicit DataSpace(DataspaceType space_type);

    /// \brief Create a scalar DataSpace.
    ///
    /// \code{.cpp}
    /// auto dataspace = DataSpace::Scalar();
    /// \endcode
    ///
    /// \since 2.9
    static DataSpace Scalar();

    /// \brief Create a null DataSpace.
    ///
    /// \code{.cpp}
    /// auto dataspace = DataSpace::Null();
    /// \endcode
    ///
    /// \since 2.9
    static DataSpace Null();

    /// \brief Create a copy of the DataSpace which will have different id.
    ///
    /// \code{.cpp}
    /// DataSpace dspace1(1, 3);
    /// auto dspace2 = dspace.clone();
    /// \endcode
    ///
    /// \since 1.0
    DataSpace clone() const;

    /// \brief Returns the number of dimensions of a DataSpace.
    /// \code{.cpp}
    /// DataSpace dspace(1, 3);
    /// size_t number_of_dim = dspace.getNumberDimensions(); // returns 2
    /// \endcode
    /// \since 1.0
    size_t getNumberDimensions() const;

    /// \brief Returns the size of the dataset in each dimension.
    ///
    /// For zero-dimensional datasets (e.g. scalar or null datasets) an empty
    /// vector is returned.
    ///
    /// \code{.cpp}
    /// DataSpace dspace(1, 3);
    /// auto dims = dspace.getDimensions(); // returns {1, 3}
    /// \endcode
    ///
    /// \sa DataSpace::getMaxDimensions
    ///
    /// \since 1.0
    std::vector<size_t> getDimensions() const;

    /// \brief Return the number of elements in this DataSpace.
    ///
    /// \code{.cpp}
    /// DataSpace dspace(1, 3);
    /// size_t elementcount = dspace.getElementCount(); // return 1 x 3 = 3
    /// \endcode
    /// \since 2.1
    size_t getElementCount() const;

    /// \brief Returns the maximum size of the dataset in each dimension.
    ///
    /// This is the maximum size a dataset can be extended to, which may be
    /// different from the current size of the dataset.
    ///
    /// \code{.cpp}
    /// DataSpace dspace(std::vector<size_t>{1, 3}, std::vector<size_t>{UNLIMITED, 10});
    /// dspace.getMaxDimensions(); // Return {UNLIMITED, 10}
    /// \endcode
    ///
    /// \sa DataSpace::getDimensions
    /// \since 2.0
    std::vector<size_t> getMaxDimensions() const;

    /// \brief Automatically deduce the DataSpace from a container/value.
    ///
    /// Certain containers and scalar values are fully supported by HighFive.
    /// For these containers, HighFive can deduce the dimensions from `value`.
    ///
    /// \code{.cpp}
    /// double d = 42.0;
    /// std::vector<std::vector<int>> v = {{4, 5, 6}, {7, 8, 9}};
    /// DataSpace::From(v); // A DataSpace of dimensions 2, 3.
    /// DataSpace::From(d); // A scalar dataspace.
    /// \endcode
    ///
    /// \since 1.0
    template <typename T>
    static DataSpace From(const T& value);

    /// \brief Create a DataSpace from a value of type string array.
    /// \param string_array An C-array of C-string (null-terminated).
    ///
    /// \code{.cpp}
    /// char string_array[2][10] = {"123456789", "abcdefghi"};
    /// auto dspace = DataSpace::FromCharArrayStrings(string_array); // dspace is a DataSpace of
    /// dimensions 2
    /// \endcode
    /// \since 2.2
    template <std::size_t N, std::size_t Width>
    static DataSpace FromCharArrayStrings(const char (&string_array)[N][Width]);

  protected:
    DataSpace() = default;

    static DataSpace fromId(hid_t hid) {
        DataSpace space;
        space._hid = hid;

        return space;
    }

    friend class Attribute;
    friend class File;
    friend class DataSet;

    friend DataSpace detail::make_data_space(hid_t hid);
};

}  // namespace HighFive

// We include bits right away since DataSpace is user-constructible
#include "bits/H5Dataspace_misc.hpp"
