/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <string>
#include <vector>

#include <H5Apublic.h>
#include <H5Ppublic.h>

#include "H5Attribute_misc.hpp"
#include "H5Iterables_misc.hpp"
#include "h5a_wrapper.hpp"

namespace HighFive {

template <typename Derivate>
inline Attribute AnnotateTraits<Derivate>::createAttribute(const std::string& attribute_name,
                                                           const DataSpace& space,
                                                           const DataType& dtype) {
    auto attr_id = detail::h5a_create2(static_cast<Derivate*>(this)->getId(),
                                       attribute_name.c_str(),
                                       dtype.getId(),
                                       space.getId(),
                                       H5P_DEFAULT,
                                       H5P_DEFAULT);
    return detail::make_attribute(attr_id);
}

template <typename Derivate>
template <typename Type>
inline Attribute AnnotateTraits<Derivate>::createAttribute(const std::string& attribute_name,
                                                           const DataSpace& space) {
    return createAttribute(attribute_name, space, create_and_check_datatype<Type>());
}

template <typename Derivate>
template <typename T>
inline Attribute AnnotateTraits<Derivate>::createAttribute(const std::string& attribute_name,
                                                           const T& data) {
    Attribute att =
        createAttribute(attribute_name,
                        DataSpace::From(data),
                        create_and_check_datatype<typename details::inspector<T>::base_type>());
    att.write(data);
    return att;
}

template <typename Derivate>
inline void AnnotateTraits<Derivate>::deleteAttribute(const std::string& attribute_name) {
    detail::h5a_delete(static_cast<const Derivate*>(this)->getId(), attribute_name.c_str());
}

template <typename Derivate>
inline Attribute AnnotateTraits<Derivate>::getAttribute(const std::string& attribute_name) const {
    const auto attr_id = detail::h5a_open(static_cast<const Derivate*>(this)->getId(),
                                          attribute_name.c_str(),
                                          H5P_DEFAULT);
    return detail::make_attribute(attr_id);
}

template <typename Derivate>
inline size_t AnnotateTraits<Derivate>::getNumberAttributes() const {
    int res = detail::h5a_get_num_attrs(static_cast<const Derivate*>(this)->getId());
    return static_cast<size_t>(res);
}

template <typename Derivate>
inline std::vector<std::string> AnnotateTraits<Derivate>::listAttributeNames() const {
    std::vector<std::string> names;
    details::HighFiveIterateData iterateData(names);

    size_t num_objs = getNumberAttributes();
    names.reserve(num_objs);

    detail::h5a_iterate2(static_cast<const Derivate*>(this)->getId(),
                         H5_INDEX_NAME,
                         H5_ITER_INC,
                         nullptr,
                         &details::internal_high_five_iterate<H5A_info_t>,
                         static_cast<void*>(&iterateData));

    return names;
}

template <typename Derivate>
inline bool AnnotateTraits<Derivate>::hasAttribute(const std::string& attr_name) const {
    return detail::h5a_exists(static_cast<const Derivate*>(this)->getId(), attr_name.c_str()) > 0;
}

}  // namespace HighFive
