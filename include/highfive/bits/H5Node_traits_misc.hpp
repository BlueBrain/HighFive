/*
 * Copyright (C) 2015 Adrien Devresse <adrien.devresse@epfl.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#ifndef H5NODE_TRAITS_MISC_HPP
#define H5NODE_TRAITS_MISC_HPP

#include "H5Node_traits.hpp"

#include <string>
#include <vector>

#include "../H5Attribute.hpp"
#include "../H5Exception.hpp"
#include "../H5DataSet.hpp"
#include "../H5DataSpace.hpp"
#include "../H5DataType.hpp"
#include "../H5Group.hpp"



#include <H5Apublic.h>
#include <H5Dpublic.h>
#include <H5Tpublic.h>
#include <H5Fpublic.h>
#include <H5Ppublic.h>
#include <H5Gpublic.h>

namespace HighFive{


namespace {

struct HighFiveIterateData{
    HighFiveIterateData(std::vector<std::string> & my_names) : names(my_names), err(NULL) {}

    std::vector<std::string> & names;
    std::exception* err;

    inline void throwIfError(){
        if(err){
            throw *err;
        }
    }
};

template<typename InfoType>
inline herr_t internal_high_five_iterate(hid_t id, const char *name, const InfoType *info, void *op_data) {
    (void)id;
    (void)info;

    HighFiveIterateData* data = static_cast<HighFiveIterateData*>(op_data);
    try {
      data->names.push_back(name);
      return 0;
    }
    catch (...) {
      data->err = new ObjectException("Exception during H5Iterate, abort listing");
    }
    return -1;
}

}


template <typename Derivate>
inline Attribute NodeTraits<Derivate>::createAttribute(const std::string & attribute_name, const DataSpace & space, const DataType & dtype) {
  Attribute attribute;
  if ((attribute._hid = H5Acreate2(static_cast<Derivate*>(this)->getId(), attribute_name.c_str(), dtype._hid, space._hid,
    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
    HDF5ErrMapper::ToException<AttributeException>(std::string("Unable to create the attribute \"") + attribute_name + "\":");
  }
  return attribute;
}

template <typename Derivate>
template <typename Type>
inline Attribute NodeTraits<Derivate>::createAttribute(const std::string & attribute_name, const DataSpace & space) {
  return createDataSet(attribute_name, space, AtomicType<Type>());
}


template <typename Derivate>
inline Attribute NodeTraits<Derivate>::getAttribute(const std::string & attribute_name) const {
  Attribute attribute;
  if ((attribute._hid = H5Aopen(static_cast<const Derivate*>(this)->getId(), attribute_name.c_str(), H5P_DEFAULT)) < 0) {
    HDF5ErrMapper::ToException<AttributeException>(std::string("Unable to open the attribute \"") + attribute_name + "\":");
  }
  return attribute;
}

template <typename Derivate>
inline size_t NodeTraits<Derivate>::getNumberAttributes() const {
  int res = H5Aget_num_attrs(static_cast<const Derivate*>(this)->getId());
  if (res < 0) {
    HDF5ErrMapper::ToException<AttributeException>(std::string("Unable to count attributes in existing group or file"));
  }
  return res;
}

template <typename Derivate>
inline std::vector<std::string> NodeTraits<Derivate>::listAttributeNames() const {

  std::vector<std::string> names;
  HighFiveIterateData iterateData(names);


  size_t num_objs = getNumberAttributes();
  names.reserve(num_objs);

  if (H5Aiterate2(static_cast<const Derivate*>(this)->getId(), H5_INDEX_NAME,
    H5_ITER_INC, NULL, &internal_high_five_iterate<H5A_info_t>, static_cast<void*>(&iterateData)) < 0) {
    HDF5ErrMapper::ToException<AttributeException>(std::string("Unable to list attributes in group"));
  }

  return names;
}

template <typename Derivate>
inline DataSet NodeTraits<Derivate>::createDataSet(const std::string & dataset_name, const DataSpace & space, const DataType & dtype){
    DataSet set;
    if( (set._hid = H5Dcreate2(static_cast<Derivate*>(this)->getId(), dataset_name.c_str(), dtype._hid, space._hid,
                              H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0){
        HDF5ErrMapper::ToException<DataSetException>(std::string("Unable to create the dataset \"")+ dataset_name+ "\":");
    }
    return set;
}

template <typename Derivate>
template <typename Type>
inline DataSet NodeTraits<Derivate>::createDataSet(const std::string & dataset_name, const DataSpace & space){
    return createDataSet(dataset_name, space, AtomicType<Type>());
}


template <typename Derivate>
inline DataSet NodeTraits<Derivate>::getDataSet(const std::string & dataset_name) const{
    DataSet set;
    if( (set._hid = H5Dopen2(static_cast<const Derivate*>(this)->getId(), dataset_name.c_str(), H5P_DEFAULT)) < 0){
        HDF5ErrMapper::ToException<DataSetException>(std::string("Unable to open the dataset \"")+ dataset_name + "\":");
    }
    return set;
}

template <typename Derivate>
inline Group NodeTraits<Derivate>::createGroup(const std::string & group_name){
    Group group;
    if( (group._hid = H5Gcreate2(static_cast<Derivate*>(this)->getId(), group_name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0){
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to create the group \"")+ group_name + "\":");
    }
    return group;
}



template <typename Derivate>
inline Group NodeTraits<Derivate>::getGroup(const std::string & group_name) const{
    Group group;
    if( (group._hid = H5Gopen2(static_cast<const Derivate*>(this)->getId(), group_name.c_str(), H5P_DEFAULT)) < 0){
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to open the group \"")+ group_name + "\":");
    }
    return group;
}


template <typename Derivate>
inline size_t NodeTraits<Derivate>::getNumberObjects() const{
    hsize_t res;
    if ( H5Gget_num_objs(static_cast<const Derivate*>(this)->getId(), &res) < 0){
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to count objects in existing group or file"));
    }
    return res;
}



template <typename Derivate>
inline std::vector<std::string> NodeTraits<Derivate>::listObjectNames() const{

    std::vector<std::string> names;
    HighFiveIterateData iterateData(names);


    size_t num_objs = getNumberObjects();
    names.reserve(num_objs);

    if( H5Literate(static_cast<const Derivate*>(this)->getId(), H5_INDEX_NAME,
                   H5_ITER_INC, NULL, &internal_high_five_iterate<H5L_info_t>, static_cast<void*>(&iterateData)) < 0){
         HDF5ErrMapper::ToException<GroupException>(std::string("Unable to list objects in group"));
    }

    return names;
}

}

#endif // H5NODE_TRAITS_MISC_HPP
