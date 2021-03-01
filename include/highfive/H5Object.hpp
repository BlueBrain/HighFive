/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5OBJECT_HPP
#define H5OBJECT_HPP

#include <ctime>

#include <H5Ipublic.h>
#include <H5Opublic.h>

#include "H5Exception.hpp"
#include "bits/H5_definitions.hpp"

namespace HighFive {

class LinkAccessProps;

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

///
/// \brief The possible types of group entries (link concept)
///
enum class LinkType {
  Hard,
  Soft,
  External,
  Other  // Reserved or User-defined
};


class Object {
public:
  // decrease reference counter
  ~Object();

  ///
  /// \brief isValid
  /// \return true if current Object is a valid HDF5Object
  ///
  bool isValid() const noexcept;

  ///
  /// \brief refresh usually you don't need to use it.
  /// Close and reopen the object
  /// \return
  ///
  void refresh() const;

  ///
  /// \brief getId
  /// \return internal HDF5 id to the object
  ///  provided for C API compatibility
  ///
  hid_t getId(const bool& increaseRefCount) const noexcept;

  ///
  /// \brief getFileId return file's ID the object belong to
  /// \param increaseRefCount if true - reopen file (you need to
  /// manually decrease ref count for this ID after you are done with this ID);
  /// false - simply create a copy of current ID (ID is valid
  /// if file is already opened). HDF5 allow to work with object
  /// while file is closed
  /// \return
  ///
  hid_t getFileId(const bool& increaseRefCount) const;
  std::string getFileName() const;

  ///
  /// \brief return the path to the current group, dataset,
  /// datatype or attribute's holder
  /// \return the path to the dataset
  std::string getPath() const;

  int getIdRefCount() const noexcept;

  ///
  /// \brief Retrieve several infos about the current object (address, dates, etc)
  ///
  ObjectInfo getObjectInfo() const;

  ///
  /// \brief Gets the fundamental type of the object (dataset, group, etc)
  /// \exception ObjectException when the _hid is negative or the type
  ///     is custom and not registered yet
  ///
  ObjectType getObjectType() const;

protected:
  // empty constructor
  Object();

  // copy constructor, increase reference counter
  Object(const Object& other);

  // move constructor, reuse hid
  Object(Object&& other) noexcept;

  // Init with an low-level object id
  explicit Object(const hid_t&);
  explicit Object(const hid_t&, const ObjectType&, const bool&);

  ///
  /// \brief getLinkInfo Cant't be applied to file or an exception appears.
  /// That is why it is protected
  /// \return
  ///
  LinkInfo getLinkInfo() const;

  Object& operator=(const Object& other);

  hid_t _hid;

private:

  template <typename Derivate> friend class NodeTraits;
  template <typename Derivate> friend class AnnotateTraits;
  friend class Reference;
  friend class DataSpace;
  friend class File;
  friend class Group;
  friend class DataSet;
  friend class Attribute;
  friend class DataType;
};


///
/// \brief A class for accessing hdf5 objects info
///
class ObjectInfo  {
public:
  /// \brief Retrieve the address of the object (within its file)
  H5_DEPRECATED("Deprecated since HighFive 2.2. Soon supporting VOL tokens")
  haddr_t getAddress() const noexcept;

  /// \brief Retrieve the number of references to this object
  size_t getHardLinkRefCount() const noexcept;

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

class LinkInfo  {
public:
  LinkType getLinkType() const noexcept;
  hbool_t creationOrderValid() const noexcept;
  int64_t getCreationOrder() const noexcept;
  H5T_cset_t getLinkNameCharacterSet() const noexcept;
#if (H5Lget_info_vers < 2)
  haddr_t getAddress() const noexcept;
#else
  /// \brief getToken Token of location that hard link points to
  H5O_token_t getHardLinkToken() const noexcept;
#endif
  /// \brief getSoftLinkSize Size of a soft link or UD link value
  size_t getSoftLinkSize() const noexcept;

protected:
#if (H5Lget_info_vers < 2)
  H5L_info_t link_info;
#else
  H5L_info2_t link_info;
#endif

  friend class Object;
};

}  // namespace HighFive

#include "bits/H5Object_misc.hpp"

#endif // H5OBJECT_HPP
