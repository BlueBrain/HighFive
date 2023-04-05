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
/// \brief Class representing an attribute of a dataset or group
///
class Attribute: public Object, public PathTraits<Attribute> {
  public:
    const static ObjectType type = ObjectType::Attribute;

    ///
    /// \brief return the name of the current attribute
    /// \return the name of the attribute
    std::string getName() const;

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
    /// \return same than getSpace for DataSet, compatibility with Selection
    /// class
    ///
    DataSpace getMemSpace() const;

    /// \brief Return the attribute
    template <typename T>
    T read() const;

    ///
    /// Read the attribute into a buffer
    /// An exception is raised if the numbers of dimension of the buffer and of
    /// the attribute are different
    ///
    /// The array type can be a N-pointer or a N-vector ( e.g int** integer two
    /// dimensional array )
    template <typename T>
    void read(T& array) const;

    /// \brief Read the attribute into a buffer
    template <typename T>
    void read(T* array, const DataType& dtype = {}) const;

    ///
    /// Write the integrality N-dimension buffer to this attribute
    /// An exception is raised if the numbers of dimension of the buffer and of
    /// the attribute are different
    ///
    /// The array type can be a N-pointer or a N-vector ( e.g int** integer two
    /// dimensional array )
    template <typename T>
    void write(const T& buffer);

    /// \brief Write a buffer to this attribute
    template <typename T>
    void write_raw(const T* buffer, const DataType& dtype = {});

    /// \brief Get the list of properties for creation of this attribute
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
