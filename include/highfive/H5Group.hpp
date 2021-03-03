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
    const static ObjectType type = ObjectType::Group;

  static Group FromId(const hid_t& id) {
    Object obj = Object(id, ObjectType::Group);
    return Group(obj);
  };
  
  protected:
    using Object::Object;

    inline Group(Object&& o) noexcept : Object(std::move(o)) {};

    Group(const Object& obj) : Object(obj){};

    friend class File;
    friend class Reference;
    template <typename Derivate> friend class ::HighFive::NodeTraits;
};

}  // namespace HighFive

#endif // HIGHFIVE_H5GROUP_HPP
