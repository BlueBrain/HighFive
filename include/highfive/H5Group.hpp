/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <H5Gpublic.h>

#include "H5Object.hpp"
#include "bits/H5Friends.hpp"
#include "bits/H5_definitions.hpp"
#include "bits/H5Annotate_traits.hpp"
#include "bits/H5Node_traits.hpp"
#include "bits/H5Path_traits.hpp"

namespace HighFive {

namespace detail {
/// \brief Internal hack to create an `Group` from an ID.
///
/// WARNING: Creating an Group from an ID has implications w.r.t. the lifetime of the object
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
Group make_group(hid_t);
}  // namespace detail

///
/// \brief Represents an hdf5 group
class Group: public Object,
             public NodeTraits<Group>,
             public AnnotateTraits<Group>,
             public PathTraits<Group> {
  public:
    const static ObjectType type = ObjectType::Group;

    Group() = default;

    std::pair<unsigned int, unsigned int> getEstimatedLinkInfo() const;

    /// \brief Get the list of properties for creation of this group
    GroupCreateProps getCreatePropertyList() const {
        return details::get_plist<GroupCreateProps>(*this, H5Gget_create_plist);
    }

    Group(Object&& o) noexcept
        : Object(std::move(o)){};

  protected:
    using Object::Object;

    friend Group detail::make_group(hid_t);
    friend class File;
    friend class Reference;
#if HIGHFIVE_HAS_FRIEND_DECLARATIONS
    template <typename Derivate>
    friend class ::HighFive::NodeTraits;
#endif
};

inline std::pair<unsigned int, unsigned int> Group::getEstimatedLinkInfo() const {
    auto gcpl = getCreatePropertyList();
    auto eli = EstimatedLinkInfo(gcpl);
    return std::make_pair(eli.getEntries(), eli.getNameLength());
}

namespace detail {
inline Group make_group(hid_t hid) {
    return Group(hid);
}
}  // namespace detail

}  // namespace HighFive
