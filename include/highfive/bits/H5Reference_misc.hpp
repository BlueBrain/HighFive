/*
 *  Copyright (c), 2020, EPFL - Blue Brain Project
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#pragma once

#include <string>
#include <H5Ppublic.h>

#include "H5Utils.hpp"

#include "../H5Object.hpp"

#include "h5r_wrapper.hpp"

namespace HighFive {

inline Reference::Reference(const Object& location, const Object& object)
    : parent_id(location.getId()) {
    obj_name = details::get_name([&](char* buffer, size_t length) {
        return detail::h5i_get_name(object.getId(), buffer, length);
    });
}

inline void Reference::create_ref(hobj_ref_t* refptr) const {
    detail::h5r_create(refptr, parent_id, obj_name.c_str(), H5R_OBJECT, -1);
}

inline ObjectType Reference::getType(const Object& location) const {
    return get_ref(location).getType();
}

template <typename T>
inline T Reference::dereference(const Object& location) const {
    static_assert(std::is_same<DataSet, T>::value || std::is_same<Group, T>::value,
                  "We can only (de)reference HighFive::Group or HighFive:DataSet");
    auto obj = get_ref(location);
    if (obj.getType() != T::type) {
        HDF5ErrMapper::ToException<ReferenceException>("Trying to dereference the wrong type");
    }
#if defined __GNUC__ && __GNUC__ < 9
    return std::move(obj);
#else
    return obj;
#endif
}

inline Object Reference::get_ref(const Object& location) const {
#if (H5Rdereference_vers == 2)
    hid_t res = detail::h5r_dereference(location.getId(), H5P_DEFAULT, H5R_OBJECT, &href);
#else
    hid_t res = detail::h5r_dereference(location.getId(), H5R_OBJECT, &href);
#endif
    return Object(res);
}

}  // namespace HighFive
