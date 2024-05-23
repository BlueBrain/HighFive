/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <ctime>

#include <H5Ipublic.h>
#include <H5Opublic.h>

#include "bits/H5_definitions.hpp"
#include "bits/H5Friends.hpp"

namespace HighFive {

///
/// \brief Enum of the types of objects (H5O api)
///
enum class ObjectType {
    File,
    Group,
    UserDataType,
    DataSpace,
    Dataset,
    Attribute,
    Other  // Internal/custom object type
};


class Object {
  public:
    // move constructor, reuse hid
    Object(Object&& other) noexcept;

    ///
    /// \brief isValid
    /// \return true if current Object is a valid HDF5Object
    ///
    bool isValid() const noexcept;

    ///
    /// \brief getId
    /// \return internal HDF5 id to the object
    ///  provided for C API compatibility
    ///
    hid_t getId() const noexcept;

    ///
    /// \brief Retrieve several infos about the current object (address, dates, etc)
    ///
    ObjectInfo getInfo() const;

    ///
    /// \brief Gets the fundamental type of the object (dataset, group, etc)
    /// \exception ObjectException when the _hid is negative or the type
    ///     is custom and not registered yet
    ///
    ObjectType getType() const;

    // Check if refer to same object
    bool operator==(const Object& other) const noexcept {
        return _hid == other._hid;
    }

  protected:
    // empty constructor
    Object();

    // copy constructor, increase reference counter
    Object(const Object& other);

    // Init with an low-level object id
    explicit Object(hid_t);

    // decrease reference counter
    ~Object();

    // Copy-Assignment operator
    Object& operator=(const Object& other);

    hid_t _hid;

  private:
    friend class Reference;
    friend class CompoundType;

#if HIGHFIVE_HAS_FRIEND_DECLARATIONS
    template <typename Derivate>
    friend class NodeTraits;
    template <typename Derivate>
    friend class AnnotateTraits;
    template <typename Derivate>
    friend class PathTraits;
#endif
};


///
/// \brief A class for accessing hdf5 objects info
///
class ObjectInfo {
  public:
    /// \brief Retrieve the address of the object (within its file)
    /// \deprecated Deprecated since HighFive 2.2. Soon supporting VOL tokens
    H5_DEPRECATED("Deprecated since HighFive 2.2. Soon supporting VOL tokens")
    haddr_t getAddress() const noexcept;

    /// \brief Retrieve the number of references to this object
    size_t getRefCount() const noexcept;

    /// \brief Retrieve the object's creation time
    time_t getCreationTime() const noexcept;

    /// \brief Retrieve the object's last modification time
    time_t getModificationTime() const noexcept;

  protected:
#if (H5Oget_info_vers < 3)
    H5O_info_t raw_info;
#else
    // Use compat H5O_info1_t while getAddress() is supported (deprecated)
    H5O_info1_t raw_info;
#endif

    friend class Object;
};

}  // namespace HighFive

#include "bits/H5Object_misc.hpp"
