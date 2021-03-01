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

#include "../H5DataSet.hpp"
#include "../H5Group.hpp"
#include "../H5Selection.hpp"
#include "../H5Utility.hpp"
#include "H5DataSet_misc.hpp"
#include "H5Iterables_misc.hpp"
#include "H5Selection_misc.hpp"
#include "H5Slice_traits_misc.hpp"

namespace HighFive {


template <typename Derivate>
template<typename Fake>
inline DataSet
NodeTraits<Derivate>::createDataSet(const std::string& dataset_name,
                                    const DataSpace& space,
                                    const DataType& dtype,
                                    const LinkCreateProps& linkCreateProps,
                                    const DataSetCreateProps& dsetCreateProps,
                                    const DataSetAccessProps& dsetAccessProps) {
  DataSet ds{H5Dcreate2(static_cast<Derivate*>(this)->getId(false),
                        dataset_name.c_str(), dtype._hid, space._hid,
                        linkCreateProps.getId(false), dsetCreateProps.getId(false), dsetAccessProps.getId(false))};
  if (ds._hid  < 0) {
    HDF5ErrMapper::ToException<DataSetException>(
          std::string("Unable to create the dataset \"") + dataset_name +
          "\":");
  }
  return ds;
}

template <typename Derivate>
template<typename Type>
//template<typename Type,
//         typename std::enable_if<
//           std::is_arithmetic<Type>::value ||
//           std::is_enum<Type>::value ||
//           std::is_same<std::string, Type>::value ||
//           std::is_array<Type>::value>::type*>
inline DataSet
NodeTraits<Derivate>::createDataSet(const std::string& dataset_name,
                                    const DataSpace& space,
                                    const LinkCreateProps& linkCreateProps,
                                    const DataSetCreateProps& dsetCreateProps,
                                    const DataSetAccessProps& dsetAccessProps)
{
  return createDataSet(dataset_name, space,
                       create_and_check_datatype<Type>(),
                       linkCreateProps, dsetCreateProps, dsetAccessProps);
}

template <typename Derivate>
template <typename T>
inline DataSet
NodeTraits<Derivate>::createDataSet(const std::string& dataset_name,
                                    const T& data,
                                    const LinkCreateProps& linkCreateProps,
                                    const DataSetCreateProps& dsetCreateProps,
                                    const DataSetAccessProps& dsetAccessProps) {
  DataSet ds = createDataSet(
        dataset_name, DataSpace::From(data),
        create_and_check_datatype<typename details::inspector<T>::base_type>(),
        linkCreateProps, dsetCreateProps, dsetAccessProps);
  ds.write(data);
  return ds;
}

template <typename Derivate>
template <std::size_t N>
inline DataSet
NodeTraits<Derivate>::createDataSet(const std::string& dataset_name,
                                    const FixedLenStringArray<N>& data,
                                    const LinkCreateProps& linkCreateProps,
                                    const DataSetCreateProps& dsetCreateProps,
                                    const DataSetAccessProps& dsetAccessProps) {
  DataSet ds = createDataSet<char[N]>(
        dataset_name, DataSpace(data.size()), linkCreateProps, dsetCreateProps, dsetAccessProps);
  ds.write(data);
  return ds;
}

template <typename Derivate>
inline DataSet
NodeTraits<Derivate>::getDataSet(const std::string& dataset_name,
                                 const DataSetAccessProps& accessProps) const {
  DataSet ds{H5Dopen2(static_cast<const Derivate*>(this)->getId(false),
                      dataset_name.c_str(), accessProps.getId(false))};
  if (ds._hid < 0) {
    HDF5ErrMapper::ToException<DataSetException>(
          std::string("Unable to open the dataset \"") + dataset_name + "\":");
  }
  return ds;
}

template <typename Derivate>
inline DataType NodeTraits<Derivate>::getDataType(
    const std::string& dtype_name,
    const DataTypeAccessProps& dtypeAccessProps) const {
  DataType dtype{H5Topen2(static_cast<const Derivate*>(this)->getId(false),
                          dtype_name.c_str(), dtypeAccessProps.getId(false))};
  if (dtype._hid < 0) {
    HDF5ErrMapper::ToException<DataSetException>(
          std::string("Unable to open the datatype \"") + dtype_name + "\":");
  }
  return dtype;
}

template <typename Derivate>
inline Group NodeTraits<Derivate>::createGroup(const std::string& group_name,
                                               const LinkCreateProps& linkCreateProps,
                                               const GroupCreateProps& groupCreateProps,
                                               const GroupAccessProps& groupAccessProps) {
  Group group{H5Gcreate2(static_cast<Derivate*>(this)->getId(false),
                         group_name.c_str(), linkCreateProps.getId(false), groupCreateProps.getId(false), groupAccessProps.getId(false))};
  if (group._hid < 0) {
    HDF5ErrMapper::ToException<GroupException>(
          std::string("Unable to create the group \"") + group_name + "\":");
  }
  return group;
}

template <typename Derivate>
inline Group
NodeTraits<Derivate>::getGroup(const std::string& group_name,
                               const GroupAccessProps& groupAccessProps) const {
  Group group{H5Gopen2(static_cast<const Derivate*>(this)->getId(false),
                       group_name.c_str(), groupAccessProps.getId(false))};
  if (group._hid < 0) {
    HDF5ErrMapper::ToException<GroupException>(
          std::string("Unable to open the group \"") + group_name + "\":");
  }
  return group;
}

template <typename Derivate>
inline size_t NodeTraits<Derivate>::getNumberObjects() const {
  hsize_t res;
  if (H5Gget_num_objs(static_cast<const Derivate*>(this)->getId(false), &res) < 0) {
    HDF5ErrMapper::ToException<GroupException>(
          std::string("Unable to count objects in existing group or file"));
  }
  return static_cast<size_t>(res);
}

template <typename Derivate>
inline std::string NodeTraits<Derivate>::getObjectName(size_t index, const LinkAccessProps& linkAccessProps) const {
  return details::get_name([&](char* buffer, hsize_t length) {
    return H5Lget_name_by_idx(
          static_cast<const Derivate*>(this)->getId(false), ".", H5_INDEX_NAME, H5_ITER_INC,
          index, buffer, length, linkAccessProps.getId(false));
  });
}

template <typename Derivate>
inline bool NodeTraits<Derivate>::rename(const std::string& src_path,
                                         const std::string& dst_path,
                                         const LinkCreateProps& linkCreateProps,
                                         const LinkAccessProps& linkAccessProps) const {
  herr_t status = H5Lmove(static_cast<const Derivate*>(this)->getId(false), src_path.c_str(),
                          static_cast<const Derivate*>(this)->getId(false), dst_path.c_str(),
                          linkCreateProps.getId(false), linkAccessProps.getId(false));
  if (status < 0) {
    HDF5ErrMapper::ToException<GroupException>(
          std::string("Unable to move link to \"") + dst_path + "\":");
    return false;
  }
  return true;
}

template <typename Derivate>
inline std::vector<std::string> NodeTraits<Derivate>::listObjectNames() const {

  std::vector<std::string> names;
  details::HighFiveIterateData iterateData(names);

  size_t num_objs = getNumberObjects();
  names.reserve(num_objs);

  if (H5Literate(static_cast<const Derivate*>(this)->getId(false), H5_INDEX_NAME,
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
                                         const LinkAccessProps& accessProps,
                                         bool raise_errors) const {
  SilenceHDF5 silencer{!raise_errors};
  const auto val = H5Lexists(static_cast<const Derivate*>(this)->getId(false),
                             node_name.c_str(), accessProps.getId(false));
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
inline bool NodeTraits<Derivate>::exist(const std::string& group_path,
                                        const LinkAccessProps& linkAccessProps) const {
  // When there are slashes, first check everything is fine
  // so that subsequent errors are only due to missing intermediate groups
  if (group_path.find('/') != std::string::npos) {
    _exist("/", linkAccessProps);  // Shall not throw under normal circumstances
    // Unless "/" (already checked), verify path exists (not thowing errors)
    return (group_path == "/") ? true : _exist(group_path, linkAccessProps, false);
  }
  return _exist(group_path, linkAccessProps);
}

template <typename Derivate>
bool NodeTraits<Derivate>::hasObject(
    const std::string& objName, const ObjectType& objType,
              const LinkAccessProps& linkAccessProps) const
{
  if (!exist(objName, linkAccessProps))
    return false;

  if (getObjectType(objName, linkAccessProps) != objType)
    return false;

  return true;
}

template <typename Derivate>
inline void NodeTraits<Derivate>::unlink(const std::string& obj_name, const LinkAccessProps& linkAccessProps) const {
  const herr_t val = H5Ldelete(static_cast<const Derivate*>(this)->getId(false),
                               obj_name.c_str(), linkAccessProps.getId(false));
  if (val < 0) {
    HDF5ErrMapper::ToException<GroupException>(
          std::string("Invalid name for unlink() "));
  }

}

template <typename Derivate>
inline LinkType NodeTraits<Derivate>::getLinkType(const std::string& obj_name,
                                                  const LinkAccessProps& linkAccessProps) const {
  H5L_info_t linkinfo;
  if (H5Lget_info(static_cast<const Derivate*>(this)->getId(false),
                  obj_name.c_str(), &linkinfo, linkAccessProps.getId(false)) < 0
      || linkinfo.type == H5L_TYPE_ERROR) {
    HDF5ErrMapper::ToException<GroupException>(
          std::string("Unable to obtain info for link ") + obj_name);
  }
  return _convert_link_type(linkinfo.type);
}

template <typename Derivate>
inline ObjectType NodeTraits<Derivate>::getObjectType(const std::string& obj_name,
                                                      const LinkAccessProps& accessProps) const {
  return _open(obj_name, accessProps).getObjectType();
}

template <typename Derivate>
template<typename Node,
         typename std::enable_if<
           std::is_same<Node, HighFive::File>::value |
           std::is_same<Node, HighFive::Group>::value>::type*>
inline Group NodeTraits<Derivate>::createLink(
    const Node& target,
    const std::string& linkName,
    const LinkType& linkType,
    const LinkCreateProps& linkCreateProps,
    const LinkAccessProps& linkAccessProps,
    const GroupAccessProps& groupAccessProps)
{
  _createLink(target, linkName, linkType, linkCreateProps, linkAccessProps);
  return static_cast<const Derivate*>(this)->getGroup(
        linkName, groupAccessProps);
}

template <typename Derivate>
template<typename Fake>
inline DataSet NodeTraits<Derivate>::createLink(
    const DataSet& target,
    const std::string& linkName,
    const LinkType& linkType,
    const LinkCreateProps& linkCreateProps,
    const LinkAccessProps& linkAccessProps,
    const DataSetAccessProps& dsetAccessProps)
{
  _createLink(target, linkName, linkType, linkCreateProps, linkAccessProps);
  return static_cast<const Derivate*>(this)->getDataSet(linkName, dsetAccessProps);
}

template <typename Derivate>
inline Object NodeTraits<Derivate>::_open(const std::string& node_name,
                                          const LinkAccessProps& accessProps) const {
  hid_t id = H5Oopen(static_cast<const Derivate*>(this)->getId(false),
                     node_name.c_str(),
                     accessProps.getId(false));
  if (id < 0) {
    HDF5ErrMapper::ToException<GroupException>(
          std::string("Unable to open \"") + node_name + "\":");
  }
  return Object(id);
}

template <typename Derivate>
template <typename T>
inline void NodeTraits<Derivate>::_createLink(
    T& target, const std::string& linkName,
    const LinkType& linkType,
    const LinkCreateProps& linkCreateProps,
    const LinkAccessProps& linkAccessProps)
{
  herr_t status = -1;

  if (linkType == LinkType::Soft){
    status = H5Lcreate_soft(
          target.getPath().c_str(),
          static_cast<const Derivate*>(this)->getId(false),
          linkName.c_str(), linkCreateProps.getId(false), linkAccessProps.getId(false));
  } else if (linkType == LinkType::Hard){
    status = H5Lcreate_hard(
          target.getId(false),
          target.getPath().c_str(),
          static_cast<const Derivate*>(this)->getId(false),
          linkName.c_str(), linkCreateProps.getId(false), linkAccessProps.getId(false));
  } else if (linkType == LinkType::External){
    status = H5Lcreate_external(
          target.getFileName().c_str(),
          target.getPath().c_str(),
          static_cast<const Derivate*>(this)->getId(false),
          linkName.c_str(), linkCreateProps.getId(false), linkAccessProps.getId(false));
  }

  if (status < 0) {
    HDF5ErrMapper::ToException<GroupException>(
          std::string("Unable to create link"));
  }
}


}  // namespace HighFive

#endif  // H5NODE_TRAITS_MISC_HPP
