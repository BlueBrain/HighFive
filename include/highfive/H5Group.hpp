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
#include "bits/H5Annotate_traits.hpp"
#include "bits/H5Node_traits.hpp"

namespace HighFive {

class File;

///
/// \brief Represents an hdf5 group
class Group : public Object,
              public NodeTraits<Group>,
              public AnnotateTraits<Group> {
  public:

    const static ObjectType type = ObjectType::Group;

    Group();

  protected:
    explicit Group(const Object& o) : Object(o) {};

    friend class File;

    friend class ::HighFive::Reference;
};
}

#include "bits/H5Node_traits_misc.hpp"
#include "bits/H5Group_misc.hpp"
#include "H5Attribute.hpp"  // for AnnotateTraits

#endif // HIGHFIVE_H5GROUP_HPP
