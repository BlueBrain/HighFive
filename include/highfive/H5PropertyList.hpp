/*
 *  Copyright (c), 2017-2018, Adrien Devresse <adrien.devresse@epfl.ch>
 *                            Juan Hernando <juan.hernando@epfl.ch>
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5PROPERTY_LIST_HPP
#define H5PROPERTY_LIST_HPP

#include <vector>

#include <H5Ppublic.h>

#include "H5Exception.hpp"
#include "H5Object.hpp"

namespace HighFive {

///
/// \brief Types of property lists
///
enum class PropertyType : int {
  OBJECT_CREATE,
  FILE_CREATE,
  FILE_ACCESS,
  DATASET_CREATE,
  DATASET_ACCESS,
  DATASET_XFER,
  GROUP_CREATE,
  GROUP_ACCESS,
  DATATYPE_CREATE,
  DATATYPE_ACCESS,
  STRING_CREATE,
  ATTRIBUTE_CREATE,
  OBJECT_COPY,
  LINK_CREATE,
  LINK_ACCESS,
};

///
/// \brief Base HDF5 property List
///
template <PropertyType T>
class PropertyList {

  // copy and move constructors are strongly needed by pybind11

public:
  ~PropertyList() {
    if (_hid != H5P_DEFAULT) {
      H5Pclose(_hid);
    }
  }

  PropertyList(const PropertyList<T>& other) :
    _hid(other.getId(true)){};

  PropertyList& operator=(const PropertyList<T>& other){
    _hid = other.getId(true);
    return *this;
  };

  PropertyList& operator=(PropertyList&& other) noexcept{
    _hid = other._hid;
    other._hid = H5I_INVALID_HID;
    return *this;
  }

  constexpr PropertyType getObjectType() const {
    return T;
  }

  hid_t getId(const bool& increaseRefCount) const {
    if (increaseRefCount)
      H5Iinc_ref(_hid);

    return _hid;
  }

  PropertyList() noexcept{
    initializeId();
  }

  PropertyList(PropertyList&& other) noexcept :
    _hid(other.getId(false)){
    other._hid = H5I_INVALID_HID;
  }

protected:
  void setCreateIntermediateGroup(unsigned val);
  void setExternalLinkPrefix(const std::string& prefix);
  void setShuffle();
  void setDeflate(const unsigned& level);
  void setChunk(const std::vector<hsize_t>& dims);
  void setChunkCache(
      const size_t& numSlots, const size_t& cacheSize,
      const double& w0);

  void initializeId();

  hid_t _hid;
};

class LinkCreateProps : public PropertyList<PropertyType::LINK_CREATE> {
public:
  LinkCreateProps(){
    setCreateIntermediateGroup(1);  // create intermediate groups ON by default
  }

  void setCreateIntermediateGroup(unsigned val){
    PropertyList::setCreateIntermediateGroup(val);
  }
};

class LinkAccessProps : public PropertyList<PropertyType::LINK_ACCESS> {
public:
  LinkAccessProps(){}

  void setExternalLinkPrefix(const std::string& prefix){
    PropertyList::setExternalLinkPrefix(prefix);
  }
};

class FileCreateProps : public PropertyList<PropertyType::FILE_CREATE> {
public:
  FileCreateProps(){}
};

class FileAccessProps : public PropertyList<PropertyType::FILE_ACCESS> {
public:
  FileAccessProps(){}
};

class GroupCreateProps : public PropertyList<PropertyType::GROUP_CREATE> {
public:
  GroupCreateProps(){}
};

class GroupAccessProps : public PropertyList<PropertyType::GROUP_ACCESS> {
public:
  GroupAccessProps(){}

  void setExternalLinkPrefix(const std::string& prefix){
    PropertyList::setExternalLinkPrefix(prefix);
  }
};

class DataSetCreateProps : public PropertyList<PropertyType::DATASET_CREATE> {
public:
  DataSetCreateProps(){}

  void setShuffle(){
    PropertyList::setShuffle();
  }

  void setDeflate(const unsigned& level){
    PropertyList::setDeflate(level);
  }

  void setChunk(const std::initializer_list<hsize_t>& items){
    std::vector<hsize_t> dims{items};
    setChunk(dims);
  }

  template <typename... Args>
  void setChunk(hsize_t item, Args... args){
    std::vector<hsize_t> dims{item, static_cast<hsize_t>(args)...};
    setChunk(dims);
  }

  void setChunk(const std::vector<hsize_t>& dims){
    PropertyList::setChunk(dims);
  }
};

class DataSetAccessProps : public PropertyList<PropertyType::DATASET_ACCESS> {
public:
  DataSetAccessProps(){}

  void setChunkCache(
      const size_t& numSlots, const size_t& cacheSize,
      const double& w0 = static_cast<double>(H5D_CHUNK_CACHE_W0_DEFAULT))
  {
    PropertyList::setChunkCache(numSlots, cacheSize, w0);
  }

  void setExternalLinkPrefix(const std::string& prefix){
    PropertyList::setExternalLinkPrefix(prefix);
  }
};

class DataTypeCreateProps : public PropertyList<PropertyType::DATATYPE_CREATE> {
public:
  DataTypeCreateProps(){}
};

class DataTypeAccessProps : public PropertyList<PropertyType::DATATYPE_ACCESS> {
public:
  DataTypeAccessProps(){}
};

class DataTransferProps : public PropertyList<PropertyType::DATASET_XFER> {
public:
  DataTransferProps(){}
};

}  // namespace HighFive

#include "bits/H5PropertyList_misc.hpp"

#endif  // H5PROPERTY_LIST_HPP
