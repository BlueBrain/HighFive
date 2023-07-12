/*
 *  Copyright (c), 2017, Ali Can Demiralp <ali.demiralp@rwth-aachen.de>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <vector>

#include <H5Apublic.h>

#include "H5DataType.hpp"
#include "H5Object.hpp"
#include "bits/H5Friends.hpp"
#include "bits/H5Path_traits.hpp"

namespace HighFive {
class DataSpace;

namespace detail {

/// \brief Internal hack to create an `Attribute` from an ID.
///
/// WARNING: Creating an Attribute from an ID has implications w.r.t. the lifetime of the object
///          that got passed via its ID. Using this method careless opens up the suite of issues
///          related to C-style resource management, including the analog of double free, dangling
///          pointers, etc.
///
/// NOTE: This is not part of the API and only serves to work around a compiler issue in GCC which
///       prevents us from using `friend`s instead. This function should only be used for internal
///       purposes. The problematic construct is:
///
///           template<class Derived>
///           friend class SomeCRTP<Derived>;
///
/// \private
Attribute make_attribute(hid_t hid);
}  // namespace detail

///
/// \brief Class representing an Attribute of a DataSet or Group
///
/// Create Attribute with AnnotateTraits::createAttribute
///
/// Access Attribute with AnnotateTraits::getAttribute
///
/// Delete Attribute with AnnotateTraits::deleteAttribute
///
class Attribute: public Object, public PathTraits<Attribute> {
  public:
    const static ObjectType type = ObjectType::Attribute;

    /// \brief Get the name of the current Attribute.
    /// \return the name of the attribute
    /// \since 2.2.2
    /// \code{.cpp}
    /// auto attr = dset.createAttribute<std::string>("my_attribute", DataSpace::From(string_list));
    /// std::cout << attr.getName() << std::endl; // Will print "my_attribute"
    /// \endcode
    std::string getName() const;

    /// \brief Get the amount of storage that is required for the current Attribute.
    /// \return The amount of the storage.
    /// \since 1.0
    /// \code{.cpp}
    /// size_t size = dset.createAttribute<int>("foo", DataSpace(1, 2));
    /// \endcode
    size_t getStorageSize() const;

    /// \brief Get the DataType of the Attribute.
    /// \return return the datatype associated with this Attribute.
    /// \since 1.0
    /// \code{.cpp}
    /// \endcode
    DataType getDataType() const;

    /// \brief Get the DataSpace of the current Attribute.
    /// \return return the DataSpace associated with this Attribute.
    /// \since 1.0
    /// \code{.cpp}
    /// \endcode
    DataSpace getSpace() const;

    /// \brief Get the DataSpace of the current Attribute.
    /// \return return the DataSpace associated with this Attribute.
    /// \note This is an alias of getSpace().
    /// \since 1.0
    /// \code{.cpp}
    /// \endcode
    DataSpace getMemSpace() const;

    /// \brief Get the value of the Attribute.
    /// \return The value.
    /// \since 2.5.0
    /// \code{.cpp}
    /// \endcode
    template <typename T>
    T read() const;

    /// \brief Get the value of the Attribute in a buffer.
    /// 
    /// Read the attribute into a buffer
    /// An exception is raised if the numbers of dimension of the buffer and of
    /// the attribute are different
    /// The array type can be a N-pointer or a N-vector ( e.g int** integer two
    /// dimensional array )
    ///
    /// \param array A already allocated value of the right dimension.
    /// \since 1.0
    /// \code{.cpp}
    /// \endcode
    template <typename T>
    void read(T& array) const;

    /// \brief Read the attribute into a buffer.
    /// \param array The already allocated array.
    /// \param mem_datatype The DataType of the array.
    ///
    /// \note This is the shallowest wrapper around `H5Aread`.
    /// \since 2.2.2
    /// \code{.cpp}
    /// \endcode
    template <typename T>
    void read(T* array, const DataType& mem_datatype) const;

    /// \brief Read the attribute into a buffer.
    /// \param array An already allocated array.
    ///
    /// This overload deduces the memory datatype from `T`.
    /// \since 2.2.2
    /// \code{.cpp}
    /// \endcode
    template <typename T>
    void read(T* array) const;

    /// \brief Write the value into the Attribute.
    ///
    /// Write the integrality N-dimension buffer to this attribute
    /// An exception is raised if the numbers of dimension of the buffer and of
    /// the attribute are different
    ///
    /// The array type can be a N-pointer or a N-vector ( e.g int** integer two
    /// dimensional array )
    /// \since 1.0
    /// \code{.cpp}
    /// \endcode
    template <typename T>
    void write(const T& buffer);

    /// \brief Write to this attribute from `buffer`.
    /// \param buffer the buffer to write into the Attribute.
    /// \param mem_datatype The DataType of the buffer.
    ///
    /// Note that this is the shallowest wrapper around `H5Awrite`. It's useful
    /// if you need full control. If possible prefer `Attribute::write`.
    /// \since 2.2.2
    /// \code{.cpp}
    /// \endcode
    template <typename T>
    void write_raw(const T* buffer, const DataType& mem_datatype);

    /// \brief Write to this attribute from `buffer`.
    /// \param buffer The buffer to write into the attribute.
    ///
    /// This version attempts to automatically deduce the datatype
    /// of the buffer. Note, that the file datatype is already set.
    /// \since 2.2.2
    /// \code{.cpp}
    /// \endcode
    template <typename T>
    void write_raw(const T* buffer);

    /// \brief Get the list of properties for creation of this attribute
    /// \return Properties for creation of this Attribute.
    /// since 2.5.0
    /// \code{.cpp}
    /// \endcode
    AttributeCreateProps getCreatePropertyList() const {
        return details::get_plist<AttributeCreateProps>(*this, H5Aget_create_plist);
    }

    // No empty attributes
    Attribute() = delete;

  protected:
    using Object::Object;

  private:
#if HIGHFIVE_HAS_FRIEND_DECLARATIONS
    template <typename Derivate>
    friend class ::HighFive::AnnotateTraits;
#endif

    friend Attribute detail::make_attribute(hid_t);
};

namespace detail {
inline Attribute make_attribute(hid_t hid) {
    return Attribute(hid);
}
}  // namespace detail

}  // namespace HighFive
