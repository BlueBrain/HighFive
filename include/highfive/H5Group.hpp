/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef HIGHFIVE_H5GROUP_HPP
#define HIGHFIVE_H5GROUP_HPP

#include "H5Object.hpp"
#include "bits/H5_definitions.hpp"
#include "bits/H5Annotate_traits.hpp"
#include "bits/H5Node_traits.hpp"

namespace HighFive {

///
/// \brief Represents an hdf5 group
class Group : public Object,
    public NodeTraits<Group>,
    public AnnotateTraits<Group> {
public:

  // this makes available to use both
  // Object::getObjectType and NodeTraits<T>::getObjectType
  using Object::getObjectType;
  using NodeTraits<Group>::getObjectType;

  const static ObjectType type = ObjectType::Group;

  LinkInfo getLinkInfo() const {
    return Object::getLinkInfo();
  }

  ///
  /// \brief getTargetPath For soft link that returns path to target that
  /// link points to. Otherwise it works the same way as `getPath()`
  /// \param accessProp
  /// \return
  ///
  std::string getTargetPath(
      const LinkAccessProps& accessProp = LinkAccessProps()) const{
    if (getLinkInfo().getLinkType() == LinkType::Soft){
      char str[256];

      if (H5Lget_val(getId(false), getPath().c_str(),
                     &str, 255, accessProp.getId(false)) < 0){
        HDF5ErrMapper::ToException<GroupException>(
              std::string("Can't get path to which the link points to"));
      }
      return std::string{str};
    }

    return getPath();
  }

  static Group FromId(const hid_t& id, const bool& increaseRefCount){
    Object obj = Object(id, ObjectType::Group, increaseRefCount);
    return Group(obj);
  };

protected:
  Group(const Object& obj) : Object(obj){};
  using Object::Object;

  inline Group(Object&& o) noexcept : Object(std::move(o)) {};

  friend class File;
  friend class Reference;
  template <typename Derivate> friend class ::HighFive::NodeTraits;
};

}  // namespace HighFive

#endif // HIGHFIVE_H5GROUP_HPP
