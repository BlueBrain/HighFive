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
/// \brief Base Class for Property lists, providing global default
class PropertyListBase : public Object {

  public:
    PropertyListBase() noexcept;

    static const PropertyListBase& Default() noexcept {
        static const PropertyListBase plist{};
        return plist;
    }

};


///
/// \brief HDF5 property Lists
///
template <PropertyType T>
class PropertyList : public PropertyListBase {
  public:

    ///
    /// \brief return the type of this PropertyList
    constexpr PropertyType getType() const noexcept {
        return T;
    }

    ///
    /// Add a property to this property list.
    /// A property is an object which is expected to have a method with the
    /// following signature void apply(hid_t hid) const
    ///
    template <typename P>
    void add(const P& property);

    ///
    /// Return the Default property type object
    static const PropertyList<T>& Default() noexcept {
        return static_cast<const PropertyList<T>&>(PropertyListBase::Default());
    }

  protected:
    void _initializeIfNeeded();

};

typedef PropertyList<PropertyType::OBJECT_CREATE> ObjectCreateProps;
typedef PropertyList<PropertyType::FILE_CREATE> FileCreateProps;
typedef PropertyList<PropertyType::FILE_ACCESS> FileAccessProps ;
typedef PropertyList<PropertyType::DATASET_CREATE> DataSetCreateProps;
typedef PropertyList<PropertyType::DATASET_ACCESS> DataSetAccessProps;
typedef PropertyList<PropertyType::DATASET_XFER> DataTransferProps;
typedef PropertyList<PropertyType::GROUP_CREATE> GroupCreateProps;
typedef PropertyList<PropertyType::GROUP_ACCESS> GroupAccessProps;
typedef PropertyList<PropertyType::DATATYPE_CREATE> DataTypeCreateProps;
typedef PropertyList<PropertyType::DATATYPE_ACCESS> DataTypeAccessProps;
typedef PropertyList<PropertyType::STRING_CREATE> StringCreateProps;
typedef PropertyList<PropertyType::ATTRIBUTE_CREATE> AttributeCreateProps;
typedef PropertyList<PropertyType::OBJECT_COPY> ObjectCopyProps;
typedef PropertyList<PropertyType::LINK_CREATE> LinkCreateProps;
typedef PropertyList<PropertyType::LINK_ACCESS> LinkAccessProps;

///
/// RawPropertyLists are to be used when advanced H5 properties
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
    explicit Chunking(const std::vector<hsize_t>& dims)
        : _dims(dims) {}

    Chunking(const std::initializer_list<hsize_t>& items)
        : Chunking(std::vector<hsize_t>{items}) {}

    template <typename... Args>
    explicit Chunking(hsize_t item, Args... args)
        : Chunking(std::vector<hsize_t>{item, static_cast<hsize_t>(args)...}) {}

    const std::vector<hsize_t>& getDimensions() const noexcept {
        return _dims;
    }

  private:
    friend DataSetCreateProps;
    void apply(hid_t hid) const;
    const std::vector<hsize_t> _dims;
};

class Deflate {
  public:
    explicit Deflate(unsigned level)
        : _level(level) {}

  private:
    friend DataSetCreateProps;
    void apply(hid_t hid) const;
    const unsigned _level;
};

class Szip {
  public:
    explicit Szip(unsigned options_mask = H5_SZIP_EC_OPTION_MASK, 
                  unsigned pixels_per_block = H5_SZIP_MAX_PIXELS_PER_BLOCK)
        : _options_mask(options_mask)
        , _pixels_per_block(pixels_per_block)
    {}

    private:
      friend DataSetCreateProps;
      void apply(hid_t hid) const;
      const unsigned _options_mask;
      const unsigned _pixels_per_block;
};

class Shuffle {
  public:
    Shuffle() = default;

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

class CreateIntermediateGroup {
  public:
    explicit CreateIntermediateGroup(bool create=true)
        : _create(create)
    {}

  private:
    friend ObjectCreateProps;
    friend LinkCreateProps;
    void apply(hid_t hid) const;
    const bool _create;
};

}  // namespace HighFive

#include "bits/H5PropertyList_misc.hpp"

#endif  // H5PROPERTY_LIST_HPP
