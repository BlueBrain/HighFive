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
#include "bits/H5Path_traits.hpp"

namespace HighFive {

///
/// \brief Represents an hdf5 group
class Group : public Object,
              public NodeTraits<Group>,
              public AnnotateTraits<Group>,
              public PathTraits<Group> {
  public:
    const static ObjectType type = ObjectType::Group;

    H5_DEPRECATED("Default constructor creates unsafe uninitialized objects")
    Group() = default;

  protected:
    using Object::Object;

    Group(Object&& o) noexcept : Object(std::move(o)) {};

    friend class File;
    friend class Reference;
    template <typename Derivate> friend class ::HighFive::NodeTraits;
};

}  // namespace HighFive

#endif // HIGHFIVE_H5GROUP_HPP
