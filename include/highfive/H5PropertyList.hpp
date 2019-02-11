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

#include "H5Object.hpp"

#include <H5Ppublic.h>

namespace HighFive {


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
  public:
    ~PropertyList();

#ifdef H5_USE_CXX11
    PropertyList(PropertyList&& other);
    PropertyList& operator=(PropertyList&& other);
#endif
    constexpr PropertyType getType() const { return T; }

    hid_t getId() const { return _hid; }

    PropertyList();

    template <typename P>
    PropertyList(std::initializer_list<P>);

    ///
    /// Add a property to this property list.
    /// A property is an object which is expected to have a method with the
    /// following signature void apply(hid_t hid) const
    ///
    template <typename P>
    void add(const P& property);

  protected:
    void _initializeIfNeeded();

    hid_t _hid;

  private:
#ifdef H5_USE_CXX11
    PropertyList(const PropertyList<T>&) = delete;
    PropertyList& operator=(const PropertyList<T>&) = delete;
#else
    PropertyList(const PropertyList<T>&);
    PropertyList& operator=(const PropertyList<T>&);
#endif
};

typedef PropertyList<PropertyType::FILE_CREATE> FileCreateProps;
typedef PropertyList<PropertyType::FILE_ACCESS> FileAccessProps ;
typedef PropertyList<PropertyType::DATASET_CREATE> DataSetCreateProps;
typedef PropertyList<PropertyType::DATASET_ACCESS> DataSetAccessProps;
typedef PropertyList<PropertyType::DATASET_XFER> DataTransferProps;

///
/// RawPropertieLists are to be used when advanced H5 properties
/// are desired and are not part of the HighFive API.
/// Therefore this class is mainly for internal use.
template <PropertyType T>
class RawPropertyList : public PropertyList<T> {
  public:
    template <typename F, typename... Args>
    void add(const F& funct, const Args&... args);
};


class Chunking {
  public:
    Chunking(const std::vector<hsize_t>& dims)
        : _dims(dims) {}

    Chunking(std::initializer_list<hsize_t> items)
        : Chunking(std::vector<hsize_t>{items}) {}

    template <typename... Args>
    Chunking(hsize_t item, Args... args)
        : Chunking(std::vector<hsize_t>{item, static_cast<hsize_t>(args)...}) {}

    const std::vector<hsize_t>& getDimensions() const { return _dims; }

  private:
    friend DataSetCreateProps;
    void apply(hid_t hid) const;
    const std::vector<hsize_t> _dims;
};

class Deflate {
  public:
    Deflate(unsigned level)
        : _level(level) {}

  private:
    friend DataSetCreateProps;
    void apply(hid_t hid) const;
    const unsigned _level;
};

class Shuffle {
  public:
    Shuffle() {}

  private:
    friend DataSetCreateProps;
    void apply(hid_t hid) const;
};

/// Dataset access property to control chunk cache configuration.
/// Do not confuse with the similar file access property for H5Pset_cache
class Caching {
  public:
    /// https://support.hdfgroup.org/HDF5/doc/RM/H5P/H5Pset_chunk_cache.html for
    /// details.
    Caching(const size_t numSlots,
            const size_t cacheSize,
            const double w0 = static_cast<double>(H5D_CHUNK_CACHE_W0_DEFAULT))
        : _numSlots(numSlots)
        , _cacheSize(cacheSize)
        , _w0(w0) {}

  private:
    friend DataSetAccessProps;
    void apply(hid_t hid) const;
    const size_t _numSlots;
    const size_t _cacheSize;
    const double _w0;
};

}  // namespace HighFive

#include "bits/H5PropertyList_misc.hpp"

#endif  // H5PROPERTY_LIST_HPP
