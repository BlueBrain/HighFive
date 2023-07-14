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
/// List all Attribute with AnnotateTraits::listAttributeNames
///
/// Check if an Attribute exists with AnnotateTraits::hasAttribute
///
/// Delete Attribute with AnnotateTraits::deleteAttribute
///
class Attribute: public Object, public PathTraits<Attribute> {
  public:
    const static ObjectType type = ObjectType::Attribute;

    /// \brief Get the name of the current Attribute.
    /// \return the name of the attribute
    /// \code{.cpp}
    /// auto attr = dset.createAttribute<std::string>("my_attribute", DataSpace::From(string_list));
    /// std::cout << attr.getName() << std::endl; // Will print "my_attribute"
    /// \endcode
    /// \since 2.2.2
    std::string getName() const;

    /// \brief Get the amount of storage that is required for the current Attribute.
    /// \return The amount of the storage.
    /// \code{.cpp}
    /// size_t size = dset.createAttribute<int>("foo", DataSpace(1, 2)).getStorageSize();
    /// \endcode
    /// \since 1.0
    size_t getStorageSize() const;

    /// \brief Get the DataType of the Attribute.
    /// \return return the datatype associated with this Attribute.
    /// \code{.cpp}
    /// Attribute attr = dset.createAttribute<int>("foo", DataSpace(1, 2));
    /// auto dtype = attr.getDataType(); // Will be an hdf5 type deduced from int
    /// \endcode
    /// \since 1.0
    DataType getDataType() const;

    /// \brief Get the DataSpace of the current Attribute.
    /// \return return the DataSpace associated with this Attribute.
    /// \code{.cpp}
    /// Attribute attr = dset.createAttribute<int>("foo", DataSpace(1, 2));
    /// auto dspace = attr.getSpace(); // This will be a DataSpace of dimension 1 * 2
    /// \endcode
    /// \since 1.0
    DataSpace getSpace() const;

    /// \brief Get the DataSpace of the current Attribute.
    /// \return return the DataSpace associated with this Attribute.
    /// \note This is an alias of getSpace().
    ///
    /// \code{.cpp}
    /// Attribute attr = dset.createAttribute<int>("foo", DataSpace(1, 2));
    /// auto dspace = attr.getMemSpace(); // This will be a DataSpace of dimension 1 * 2
    /// \endcode
    /// \since 1.0
    DataSpace getMemSpace() const;

    /// \brief Get the value of the Attribute.
    /// \return The value.
    /// \code{.cpp}
    /// Attribute attr = dset.getAttribute("foo");
    /// // The value will contains what have been written in the attribute
    /// std::vector<int> value = attr.read<std::vector<int>>();
    /// \endcode
    /// \since 2.5.0
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
    ///
    /// \code{.cpp}
    /// Attribute attr = dset.getAttribute("foo");
    /// std::vector<int> value;
    /// attr.read(value);
    /// \endcode
    /// \since 1.0
    template <typename T>
    void read(T& array) const;

    /// \brief Read the attribute into a buffer explicitly defining its type.
    /// \param array The already allocated array.
    /// \param mem_datatype The DataType of the array.
    ///
    /// \note This is the shallowest wrapper around `H5Aread`.
    ///
    /// \code{.cpp}
    /// Attribute attr = dset.getAttribute("foo");
    /// std::vector<int> value(2); // You should allocate vector yourself before
    /// attr.read(value.data(), dtype);
    /// \endcode
    /// \since 2.2.2
    template <typename T>
    void read(T* array, const DataType& mem_datatype) const;

    /// \brief Read the attribute into a buffer.
    /// \param array An already allocated array.
    ///
    /// This overload deduces the memory datatype from `T`.
    /// \code{.cpp}
    /// Attribute attr = dset.getAttribute("foo");
    /// std::vector<int> value(2); // You should allocate vector yourself before
    /// attr.read(value.data());
    /// \endcode
    /// \since 2.2.2
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
    /// \code{.cpp}
    /// Attribute attr = dset.createAttribute<int>("foo", DataSpace(1, 2));
    /// std::vector<std::vector<int>> value{{1, 2}};
    /// attr.write(value);
    /// \endcode
    /// \since 1.0
    template <typename T>
    void write(const T& buffer);

    /// \brief Write to this attribute from `buffer`.
    /// \param buffer the buffer to write into the Attribute.
    /// \param mem_datatype The DataType of the buffer.
    ///
    /// \note This is the shallowest wrapper around `H5Awrite`. It's useful
    /// if you need full control. If possible prefer Attribute::write.
    ///
    /// \code{.cpp}
    /// Attribute attr = dset.createAttribute<int>("foo", DataSpace(1, 2));
    /// std::vector<std::vector<int>> value{{1, 2}};
    /// attr.write(value.data(), AtomicType<int>());
    /// \endcode
    /// \since 2.2.2
    template <typename T>
    void write_raw(const T* buffer, const DataType& mem_datatype);

    /// \brief Write to this attribute from `buffer`.
    /// \param buffer The buffer to write into the attribute.
    ///
    /// This version attempts to automatically deduce the datatype
    /// of the buffer. Note, that the file datatype is already set.
    /// \code{.cpp}
    /// Attribute attr = dset.createAttribute<int>("foo", DataSpace(1, 2));
    /// std::vector<std::vector<int>> value{{1, 2}};
    /// attr.write(value.data());
    /// \endcode
    /// \since 2.2.2
    template <typename T>
    void write_raw(const T* buffer);

    /// \brief Get the list of properties for creation of this attribute.
    /// \return Properties for creation of this Attribute.
    /// \code{.cpp}
    /// Attribute attr = dset.getAttribute("foo");
    /// auto props = attr.getCreatePropertyList();
    /// \endcode
    /// \since 2.5.0
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
