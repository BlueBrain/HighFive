/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5NODE_TRAITS_MISC_HPP
#define H5NODE_TRAITS_MISC_HPP

#include <string>
#include <vector>

#include <H5Apublic.h>
#include <H5Dpublic.h>
#include <H5Fpublic.h>
#include <H5Gpublic.h>
#include <H5Ppublic.h>
#include <H5Tpublic.h>

#include "H5Iterables_misc.hpp"
#include "../H5DataSet.hpp"
#include "../H5Selection.hpp"
#include "../H5Utility.hpp"

namespace HighFive {


template <typename Derivate>
inline DataSet
NodeTraits<Derivate>::createDataSet(const std::string& dataset_name,
                                    const DataSpace& space,
                                    const DataType& dtype,
                                    const DataSetCreateProps& createProps,
                                    const DataSetAccessProps& accessProps) {
    DataSet set;
    if ((set._hid = H5Dcreate2(static_cast<Derivate*>(this)->getId(),
                               dataset_name.c_str(), dtype._hid, space._hid,
                               H5P_DEFAULT, createProps.getId(),
                               accessProps.getId())) < 0) {
        HDF5ErrMapper::ToException<DataSetException>(
            std::string("Unable to create the dataset \"") + dataset_name +
            "\":");
    }
    return set;
}

template <typename Derivate>
template <typename Type>
inline DataSet
NodeTraits<Derivate>::createDataSet(const std::string& dataset_name,
                                    const DataSpace& space,
                                    const DataSetCreateProps& createProps,
                                    const DataSetAccessProps& accessProps)
{
    return createDataSet(dataset_name, space,
                         create_and_check_datatype<Type>(),
                         createProps, accessProps);
}

template <typename Derivate>
template <typename T>
inline DataSet
NodeTraits<Derivate>::createDataSet(const std::string& dataset_name,
                                    const T& data,
                                    const DataSetCreateProps& createProps,
                                    const DataSetAccessProps& accessProps) {
    DataSet ds = createDataSet(
        dataset_name, DataSpace::From(data),
        create_and_check_datatype<typename details::type_of_array<T>::type>(),
        createProps, accessProps);
    ds.write(data);
    return ds;
}

template <typename Derivate>
template <std::size_t N>
inline DataSet
NodeTraits<Derivate>::createDataSet(const std::string& dataset_name,
                                    const FixedLenStringArray<N>& data,
                                    const DataSetCreateProps& createProps,
                                    const DataSetAccessProps& accessProps) {
    DataSet ds = createDataSet<char[N]>(
        dataset_name, DataSpace(data.size()), createProps, accessProps);
    ds.write(data);
    return ds;
}

template <typename Derivate>
inline DataSet
NodeTraits<Derivate>::getDataSet(const std::string& dataset_name,
                                 const DataSetAccessProps& accessProps) const {
    DataSet set;
    if ((set._hid = H5Dopen2(static_cast<const Derivate*>(this)->getId(),
                             dataset_name.c_str(), accessProps.getId())) < 0) {
        HDF5ErrMapper::ToException<DataSetException>(
            std::string("Unable to open the dataset \"") + dataset_name + "\":");
    }
    return set;
}

template <typename Derivate>
inline Group NodeTraits<Derivate>::createGroup(const std::string& group_name,
                                               bool parents) {
    RawPropertyList<PropertyType::LINK_CREATE> lcpl;
    if (parents) {
        lcpl.add(H5Pset_create_intermediate_group, 1u);
    }
    Group group;
    if ((group._hid = H5Gcreate2(static_cast<Derivate*>(this)->getId(),
                                 group_name.c_str(), lcpl.getId(), H5P_DEFAULT,
                                 H5P_DEFAULT)) < 0) {
        HDF5ErrMapper::ToException<GroupException>(
            std::string("Unable to create the group \"") + group_name + "\":");
    }
    return group;
}

template <typename Derivate>
inline Group
NodeTraits<Derivate>::getGroup(const std::string& group_name) const {
    Group group;
    if ((group._hid = H5Gopen2(static_cast<const Derivate*>(this)->getId(),
                               group_name.c_str(), H5P_DEFAULT)) < 0) {
        HDF5ErrMapper::ToException<GroupException>(
            std::string("Unable to open the group \"") + group_name + "\":");
    }
    return group;
}

template <typename Derivate>
inline size_t NodeTraits<Derivate>::getNumberObjects() const {
    hsize_t res;
    if (H5Gget_num_objs(static_cast<const Derivate*>(this)->getId(), &res) < 0) {
        HDF5ErrMapper::ToException<GroupException>(
            std::string("Unable to count objects in existing group or file"));
    }
    return static_cast<size_t>(res);
}

template <typename Derivate>
inline std::string NodeTraits<Derivate>::getObjectName(size_t index) const {
    const size_t maxLength = 255;
    char buffer[maxLength + 1];
    ssize_t retcode = H5Lget_name_by_idx(
        static_cast<const Derivate*>(this)->getId(), ".", H5_INDEX_NAME, H5_ITER_INC,
        index, buffer, static_cast<hsize_t>(maxLength) + 1, H5P_DEFAULT);
    if (retcode < 0) {
        HDF5ErrMapper::ToException<GroupException>("Error accessing object name");
    }
    const size_t length = static_cast<std::size_t>(retcode);
    if (length <= maxLength) {
        return std::string(buffer, length);
    }
    std::vector<char> bigBuffer(length + 1, 0);
    H5Lget_name_by_idx(
        static_cast<const Derivate*>(this)->getId(), ".", H5_INDEX_NAME, H5_ITER_INC,
        index, bigBuffer.data(), static_cast<hsize_t>(length) + 1, H5P_DEFAULT);
    return std::string(bigBuffer.data(), length);
}

template <typename Derivate>
inline std::vector<std::string> NodeTraits<Derivate>::listObjectNames() const {

    std::vector<std::string> names;
    details::HighFiveIterateData iterateData(names);

    size_t num_objs = getNumberObjects();
    names.reserve(num_objs);

    if (H5Literate(static_cast<const Derivate*>(this)->getId(), H5_INDEX_NAME,
                   H5_ITER_INC, NULL,
                   &details::internal_high_five_iterate<H5L_info_t>,
                   static_cast<void*>(&iterateData)) < 0) {
        HDF5ErrMapper::ToException<GroupException>(
            std::string("Unable to list objects in group"));
    }

    return names;
}

template <typename Derivate>
inline bool NodeTraits<Derivate>::_exist(const std::string& node_name,
                                         bool raise_errors) const {
    SilenceHDF5 silencer{raise_errors};
    const auto val = H5Lexists(static_cast<const Derivate*>(this)->getId(),
                               node_name.c_str(), H5P_DEFAULT);
    if (val < 0) {
        if (raise_errors) {
            HDF5ErrMapper::ToException<GroupException>("Invalid link for exist()");
        } else {
            return false;
        }
    }

    // The root path always exists, but H5Lexists return 0 or 1
    // depending of the version of HDF5, so always return true for it
    // We had to call H5Lexists anyway to check that there are no errors
    return (node_name == "/") ? true : (val > 0);
}

template <typename Derivate>
inline bool NodeTraits<Derivate>::exist(const std::string& group_path) const {
    // When there are slashes, first check everything is fine
    // so that subsequent errors are only due to missing intermediate groups
    if (group_path.find('/') != std::string::npos) {
        _exist("/");  // Shall not throw under normal circumstances
        // Unless "/" (already checked), verify path exists (not thowing errors)
        return (group_path == "/") ? true : _exist(group_path, false);
    }
    return _exist(group_path);
}


template <typename Derivate>
inline void NodeTraits<Derivate>::unlink(const std::string& node_name) const {
    const herr_t val = H5Ldelete(static_cast<const Derivate*>(this)->getId(),
                                 node_name.c_str(), H5P_DEFAULT);
    if (val < 0) {
        HDF5ErrMapper::ToException<GroupException>(
            std::string("Invalid name for unlink() "));
    }

}



// convert internal link types to enum class.
// This function is internal, so H5L_TYPE_ERROR shall be handled in the calling context
static inline LinkType _convert_link_type(const H5L_type_t& ltype) noexcept {
    switch (ltype) {
        case H5L_TYPE_HARD:
            return LinkType::Hard;
        case H5L_TYPE_SOFT:
            return LinkType::Soft;
        case H5L_TYPE_EXTERNAL:
            return LinkType::External;
        default:
            // Other link types are possible but are considered strange to HighFive.
            // see https://support.hdfgroup.org/HDF5/doc/RM/H5L/H5Lregister.htm
            return LinkType::Other;
    }
}

template <typename Derivate>
inline LinkType NodeTraits<Derivate>::getLinkType(const std::string& node_name) const {
    H5L_info_t linkinfo;
    if (H5Lget_info(static_cast<const Derivate*>(this)->getId(),
                    node_name.c_str(), &linkinfo, H5P_DEFAULT) < 0
            || linkinfo.type == H5L_TYPE_ERROR) {
        HDF5ErrMapper::ToException<GroupException>(
            std::string("Unable to obtain info for link ") + node_name);
    }
    return _convert_link_type(linkinfo.type);
}

template <typename Derivate>
inline ObjectType NodeTraits<Derivate>::getObjectType(const std::string& node_name) const {
    return _open(node_name).getType();
}


template <typename Derivate>
inline Object NodeTraits<Derivate>::_open(const std::string& node_name,
                                          const DataSetAccessProps& accessProps) const {
    hid_t id = H5Oopen(static_cast<const Derivate*>(this)->getId(),
                       node_name.c_str(),
                       accessProps.getId());
    if (id < 0) {
        HDF5ErrMapper::ToException<GroupException>(
            std::string("Unable to open \"") + node_name + "\":");
    }
    return Object(id);
}



}  // namespace HighFive

#endif  // H5NODE_TRAITS_MISC_HPP
