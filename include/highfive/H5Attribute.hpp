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

/// \brief Class representing an Attribute of a DataSet or Group
///
/// \sa AnnotateTraits::createAttribute, AnnotateTraits::getAttribute, AnnotateTraits::listAttributeNames, AnnotateTraits::hasAttribute, AnnotateTraits::deleteAttribute for create, get, list, check or delete Attribute
class Attribute: public Object, public PathTraits<Attribute> {
  public:
    const static ObjectType type = ObjectType::Attribute;

    /// \brief Get the name of the current Attribute.
    /// \code{.cpp}
    /// auto attr = dset.createAttribute<std::string>("my_attribute", DataSpace::From(string_list));
    /// std::cout << attr.getName() << std::endl; // Will print "my_attribute"
    /// \endcode
    /// \since 2.2.2
    std::string getName() const;

    /// \brief The number of bytes required to store the attribute in the HDF5 file.
    /// \code{.cpp}
    /// size_t size = dset.createAttribute<int>("foo", DataSpace(1, 2)).getStorageSize();
    /// \endcode
    /// \since 1.0
    size_t getStorageSize() const;

    /// \brief Get the DataType of the Attribute.
    /// \code{.cpp}
    /// Attribute attr = dset.createAttribute<int>("foo", DataSpace(1, 2));
    /// auto dtype = attr.getDataType(); // Will be an hdf5 type deduced from int
    /// \endcode
    /// \since 1.0
    DataType getDataType() const;

    /// \brief Get the DataSpace of the current Attribute.
    /// \code{.cpp}
    /// Attribute attr = dset.createAttribute<int>("foo", DataSpace(1, 2));
    /// auto dspace = attr.getSpace(); // This will be a DataSpace of dimension 1 * 2
    /// \endcode
    /// \since 1.0
    DataSpace getSpace() const;

    /// \brief Get the DataSpace of the current Attribute.
    /// \note This is an alias of getSpace().
    /// \since 1.0
    DataSpace getMemSpace() const;

    /// \brief Get the value of the Attribute.
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
    /// Read the attribute into an existing object. Only available for
    /// supported types `T`. If `array` has preallocated the correct amount of
    /// memory, then this routine should not trigger reallocation. Otherwise,
    /// if supported, the object will be resized.
    ///
    /// An exception is raised if the numbers of dimension of the buffer and of
    /// the attribute are different.
    ///
    /// \code{.cpp}
    /// // Will read into `value` avoiding memory allocation if the dimensions
    /// // match, i.e. if the attribute `"foo"` has three element.
    /// std::vector<int> value(3);
    /// file.getAttribute("foo").read(value);
    /// \endcode
    /// \since 1.0
    template <typename T>
    void read(T& array) const;

    /// \brief Read the attribute into a pre-allocated buffer.
    /// \deprecated use `read(T&` or `read_raw`.
    template <typename T>
    void read(T* array, const DataType& mem_datatype) const;

    /// \brief Read the attribute into a buffer.
    /// \deprecated use `read(T&` or `read_raw`.
    template <typename T>
    void read(T* array) const;

    /// \brief Read the attribute into a pre-allocated buffer.
    /// \param array A pointer to the first byte of sufficient pre-allocated memory.
    /// \param mem_datatype The DataType of the array.
    ///
    /// \note This is the shallowest wrapper around `H5Aread`. If possible
    /// prefer either Attribute::read() const or Attribute::read(T&) const.
    ///
    /// \code{.cpp}
    /// auto attr = file.getAttribute("foo");
    ///
    /// // Simulate custom allocation by the application.
    /// size_t n_elements = attr.getSpace().getElementCount();
    /// int * ptr = (int*) malloc(n_elements*sizeof(int));
    ///
    /// // Read into the pre-allocated memory.
    /// attr.read(ptr, mem_datatype);
    /// \endcode
    /// \since 2.2.2
    template <typename T>
    void read_raw(T* array, const DataType& mem_datatype) const;

    /// \brief Read the attribute into a buffer.
    /// Behaves like Attribute::read(T*, const DataType&) const but
    /// additionally this overload deduces the memory datatype from `T`.
    ///
    /// \param array Pointer to the first byte of pre-allocated memory.
    ///
    /// \note If possible prefer either Attribute::read() const or Attribute::read(T&) const.
    ///
    /// \code{.cpp}
    /// auto attr = file.getAttribute("foo");
    ///
    /// // Simulate custom allocation by the application.
    /// size_t n_elements = attr.getSpace().getElementCount();
    /// int * ptr = (int*) malloc(n_elements*sizeof(int));
    ///
    /// // Read into the pre-allocated memory.
    /// attr.read(ptr);
    /// \endcode
    /// \since 2.2.2
    template <typename T>
    void read_raw(T* array) const;

    /// \brief Write the value into the Attribute.
    ///
    /// Write the value to the attribute. For supported types `T`, this overload
    /// will write the value to the attribute. The datatype and dataspace are
    /// deduced automatically. However, since the attribute has already been
    /// created, the dimensions of `value` must match those of the attribute.
    ///
    /// \code{.cpp}
    /// // Prefer the fused version if creating and writing the attribute
    /// // at the same time.
    /// dset.createAttribute("foo", std::vector<int>{1, 2, 3});
    ///
    /// // To overwrite the value:
    /// std::vector<int> value{4, 5, 6};
    /// dset.getAttribute<int>("foo").write(value);
    /// \endcode
    /// \since 1.0
    template <typename T>
    void write(const T& value);

    /// \brief Write from a raw pointer.
    ///
    /// Values that have been correctly arranged memory, can be written directly
    /// by passing a raw pointer.
    ///
    /// \param buffer Pointer to the first byte of the value.
    /// \param mem_datatype The DataType of the buffer.
    ///
    /// \note This is the shallowest wrapper around `H5Awrite`. It's useful
    /// if you need full control. If possible prefer Attribute::write.
    ///
    /// \code{.cpp}
    /// Attribute attr = dset.createAttribute<int>("foo", DataSpace(2, 3));
    ///
    /// // Simulate the application creating `value` and only exposing access
    /// // to the raw pointer `ptr`.
    /// std::vector<std::array<int, 3>> value{{1, 2, 3}, {4, 5, 6}};
    /// int * ptr = (int*) value.data();
    ///
    /// // Simply write the bytes to disk.
    /// attr.write(ptr, AtomicType<int>());
    /// \endcode
    /// \since 2.2.2
    template <typename T>
    void write_raw(const T* buffer, const DataType& mem_datatype);

    /// \brief Write from a raw pointer.
    ///
    /// Much like Attribute::write_raw(const T*, const DataType&).
    /// Additionally, this overload attempts to automatically deduce the
    /// datatype of the buffer. Note, that the file datatype is already set.
    ///
    /// \param buffer Pointer to the first byte.
    ///
    /// \note If possible prefer Attribute::write.
    ///
    /// \code{.cpp}
    /// // Simulate the application creating `value` and only exposing access
    /// // to the raw pointer `ptr`.
    /// std::vector<std::array<int, 3>> value{{1, 2, 3}, {4, 5, 6}};
    /// int * ptr = (int*) value.data();
    ///
    /// // Simply write the bytes to disk.
    /// attr.write(ptr);
    /// \endcode
    /// \since 2.2.2
    template <typename T>
    void write_raw(const T* buffer);

    /// \brief The create property list used for this attribute.
    ///
    /// Some of HDF5 properties/setting of an attribute are defined by a
    /// create property list. This method returns a copy of the create
    /// property list used during creation of the attribute.
    ///
    /// \code{.cpp}
    /// auto acpl = attr.getCreatePropertyList();
    ///
    /// // For example to create another attribute with the same properties.
    /// file.createAttribute("foo", 42, acpl);
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
