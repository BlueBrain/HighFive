/*
 *  Copyright (c), 2017-2018, Adrien Devresse <adrien.devresse@epfl.ch>
 *                            Juan Hernando <juan.hernando@epfl.ch>
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <H5Ppublic.h>

namespace HighFive {

namespace {
inline hid_t convert_plist_type(PropertyType propertyType) {
    // The HP5_XXX are macros with function calls so we can't assign
    // them as the enum values
    switch (propertyType) {
    case PropertyType::OBJECT_CREATE:
        return H5P_OBJECT_CREATE;
    case PropertyType::FILE_CREATE:
        return H5P_FILE_CREATE;
    case PropertyType::FILE_ACCESS:
        return H5P_FILE_ACCESS;
    case PropertyType::DATASET_CREATE:
        return H5P_DATASET_CREATE;
    case PropertyType::DATASET_ACCESS:
        return H5P_DATASET_ACCESS;
    case PropertyType::DATASET_XFER:
        return H5P_DATASET_XFER;
    case PropertyType::GROUP_CREATE:
        return H5P_GROUP_CREATE;
    case PropertyType::GROUP_ACCESS:
        return H5P_GROUP_ACCESS;
    case PropertyType::DATATYPE_CREATE:
        return H5P_DATATYPE_CREATE;
    case PropertyType::DATATYPE_ACCESS:
        return H5P_DATATYPE_ACCESS;
    case PropertyType::STRING_CREATE:
        return H5P_STRING_CREATE;
    case PropertyType::ATTRIBUTE_CREATE:
        return H5P_ATTRIBUTE_CREATE;
    case PropertyType::OBJECT_COPY:
        return H5P_OBJECT_COPY;
    case PropertyType::LINK_CREATE:
        return H5P_LINK_CREATE;
    case PropertyType::LINK_ACCESS:
        return H5P_LINK_ACCESS;
    default:
        HDF5ErrMapper::ToException<PropertyException>("Unsupported property list type");
    }
}

}  // namespace


inline PropertyListBase::PropertyListBase() noexcept
    : Object(H5P_DEFAULT) {}


template <PropertyType T>
inline void PropertyList<T>::_initializeIfNeeded() {
    if (_hid != H5P_DEFAULT) {
        return;
    }
    if ((_hid = H5Pcreate(convert_plist_type(T))) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Unable to create property list");
    }
}

template <PropertyType T>
template <typename P>
inline void PropertyList<T>::add(const P& property) {
    _initializeIfNeeded();
    property.apply(_hid);
}

template <PropertyType T>
template <typename F, typename... Args>
inline void RawPropertyList<T>::add(const F& funct, const Args&... args) {
    this->_initializeIfNeeded();
    if (funct(this->_hid, args...) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting raw hdf5 property.");
    }
}

// Specific options to be added to Property Lists
#if H5_VERSION_GE(1, 10, 1)
inline FileSpaceStrategy::FileSpaceStrategy(H5F_fspace_strategy_t strategy,
                                            hbool_t persist,
                                            hsize_t threshold)
    : _strategy(strategy)
    , _persist(persist)
    , _threshold(threshold) {}

FileSpaceStrategy::FileSpaceStrategy(const FileCreateProps& fcpl) {
    if (H5Pget_file_space_strategy(fcpl.getId(), &_strategy, &_persist, &_threshold) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Unable to get file space strategy");
    }
}

inline void FileSpaceStrategy::apply(const hid_t list) const {
    if (H5Pset_file_space_strategy(list, _strategy, _persist, _threshold) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting file space strategy.");
    }
}

H5F_fspace_strategy_t FileSpaceStrategy::getStrategy() const {
    return _strategy;
}
hbool_t FileSpaceStrategy::getPersist() const {
    return _persist;
}
hsize_t FileSpaceStrategy::getThreshold() const {
    return _threshold;
}

inline FileSpacePageSize::FileSpacePageSize(hsize_t page_size)
    : _page_size(page_size) {}

inline void FileSpacePageSize::apply(const hid_t list) const {
    if (H5Pset_file_space_page_size(list, _page_size) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting file space page size.");
    }
}

FileSpacePageSize::FileSpacePageSize(const FileCreateProps& fcpl) {
    if (H5Pget_file_space_page_size(fcpl.getId(), &_page_size) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Unable to get file space page size");
    }
}

hsize_t FileSpacePageSize::getPageSize() const {
    return _page_size;
}

#ifndef H5_HAVE_PARALLEL
inline PageBufferSize::PageBufferSize(size_t page_buffer_size,
                                      unsigned min_meta_percent,
                                      unsigned min_raw_percent)
    : _page_buffer_size(page_buffer_size)
    , _min_meta(min_meta_percent)
    , _min_raw(min_raw_percent) {}

inline PageBufferSize::PageBufferSize(const FileAccessProps& plist) {
    if (H5Pget_page_buffer_size(plist.getId(), &_page_buffer_size, &_min_meta, &_min_raw) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting page buffer size.");
    }
}

inline void PageBufferSize::apply(const hid_t list) const {
    if (H5Pset_page_buffer_size(list, _page_buffer_size, _min_meta, _min_raw) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting page buffer size.");
    }
}

inline hsize_t PageBufferSize::getPageBufferSize() const {
    return _page_buffer_size;
}

inline unsigned PageBufferSize::getMinPercent() const {
    return _min_meta;
}

inline unsigned PageBufferSize::getRawPercent() const {
    return _min_raw;
}
#endif
#endif

#ifdef H5_HAVE_PARALLEL

MPIOFileAccess::MPIOFileAccess(MPI_Comm comm, MPI_Info info)
    : _comm(comm)
    , _info(info) {}

void MPIOFileAccess::apply(const hid_t list) const {
    if (H5Pset_fapl_mpio(list, _comm, _info) < 0) {
        HDF5ErrMapper::ToException<FileException>("Unable to set-up MPIO Driver configuration");
    }
}

inline void MPIOCollectiveMetadata::apply(const hid_t plist) const {
    auto read = MPIOCollectiveMetadataRead{collective_read_};
    auto write = MPIOCollectiveMetadataWrite{collective_write_};

    read.apply(plist);
    write.apply(plist);
}

MPIOCollectiveMetadata::MPIOCollectiveMetadata(bool collective)
    : collective_read_(collective)
    , collective_write_(collective) {}


MPIOCollectiveMetadata::MPIOCollectiveMetadata(const FileAccessProps& plist)
    : collective_read_(MPIOCollectiveMetadataRead(plist).isCollective())
    , collective_write_(MPIOCollectiveMetadataWrite(plist).isCollective()) {}

bool MPIOCollectiveMetadata::isCollectiveRead() const {
    return collective_read_;
}

bool MPIOCollectiveMetadata::isCollectiveWrite() const {
    return collective_write_;
}


inline void MPIOCollectiveMetadataRead::apply(const hid_t plist) const {
    if (H5Pset_all_coll_metadata_ops(plist, collective_) < 0) {
        HDF5ErrMapper::ToException<FileException>("Unable to request collective metadata reads");
    }
}

inline bool MPIOCollectiveMetadataRead::isCollective() const {
    return collective_;
}

MPIOCollectiveMetadataRead::MPIOCollectiveMetadataRead(const FileAccessProps& plist) {
    if (H5Pget_all_coll_metadata_ops(plist.getId(), &collective_) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error loading MPI metadata read.");
    }
}

MPIOCollectiveMetadataRead::MPIOCollectiveMetadataRead(bool collective)
    : collective_(collective) {}

inline void MPIOCollectiveMetadataWrite::apply(const hid_t plist) const {
    if (H5Pset_coll_metadata_write(plist, collective_) < 0) {
        HDF5ErrMapper::ToException<FileException>("Unable to request collective metadata writes");
    }
}

inline bool MPIOCollectiveMetadataWrite::isCollective() const {
    return collective_;
}

MPIOCollectiveMetadataWrite::MPIOCollectiveMetadataWrite(const FileAccessProps& plist) {
    if (H5Pget_coll_metadata_write(plist.getId(), &collective_) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error loading MPI metadata write.");
    }
}

MPIOCollectiveMetadataWrite::MPIOCollectiveMetadataWrite(bool collective)
    : collective_(collective) {}

#endif

FileVersionBounds::FileVersionBounds(H5F_libver_t low, H5F_libver_t high)
    : _low(low)
    , _high(high) {}

FileVersionBounds::FileVersionBounds(const FileAccessProps& fapl) {
    if (H5Pget_libver_bounds(fapl.getId(), &_low, &_high) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Unable to access file version bounds");
    }
}

std::pair<H5F_libver_t, H5F_libver_t> FileVersionBounds::getVersion() const {
    return std::make_pair(_low, _high);
}

void FileVersionBounds::apply(const hid_t list) const {
    if (H5Pset_libver_bounds(list, _low, _high) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting file version bounds");
    }
}

MetadataBlockSize::MetadataBlockSize(hsize_t size)
    : _size(size) {}

MetadataBlockSize::MetadataBlockSize(const FileAccessProps& fapl) {
    if (H5Pget_meta_block_size(fapl.getId(), &_size) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Unable to access file metadata block size");
    }
}

void MetadataBlockSize::apply(const hid_t list) const {
    if (H5Pset_meta_block_size(list, _size) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting metadata block size");
    }
}

hsize_t MetadataBlockSize::getSize() const {
    return _size;
}

inline void EstimatedLinkInfo::apply(const hid_t hid) const {
    if (H5Pset_est_link_info(hid, _entries, _length) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting estimated link info");
    }
}

EstimatedLinkInfo::EstimatedLinkInfo(unsigned int entries, unsigned int length)
    : _entries(entries)
    , _length(length) {}

EstimatedLinkInfo::EstimatedLinkInfo(const GroupCreateProps& gcpl) {
    if (H5Pget_est_link_info(gcpl.getId(), &_entries, &_length) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Unable to access group link size property");
    }
}

unsigned EstimatedLinkInfo::getEntries() const {
    return _entries;
}

unsigned EstimatedLinkInfo::getNameLength() const {
    return _length;
}

inline void Chunking::apply(const hid_t hid) const {
    if (H5Pset_chunk(hid, static_cast<int>(_dims.size()), _dims.data()) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting chunk property");
    }
}

Chunking::Chunking(const std::vector<hsize_t>& dims)
    : _dims(dims) {}

Chunking::Chunking(const std::initializer_list<hsize_t>& items)
    : Chunking(std::vector<hsize_t>{items}) {}

Chunking::Chunking(DataSetCreateProps& plist, size_t max_dims)
    : _dims(max_dims + 1) {
    auto n_loaded = H5Pget_chunk(plist.getId(), static_cast<int>(_dims.size()), _dims.data());
    if (n_loaded < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error getting chunk size");
    }

    if (n_loaded >= static_cast<int>(_dims.size())) {
        *this = Chunking(plist, 8 * max_dims);
    } else {
        _dims.resize(static_cast<size_t>(n_loaded));
    }
}

const std::vector<hsize_t>& Chunking::getDimensions() const noexcept {
    return _dims;
}

template <typename... Args>
Chunking::Chunking(hsize_t item, Args... args)
    : Chunking(std::vector<hsize_t>{item, static_cast<hsize_t>(args)...}) {}

inline void Deflate::apply(const hid_t hid) const {
    if (!H5Zfilter_avail(H5Z_FILTER_DEFLATE) || H5Pset_deflate(hid, _level) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting deflate property");
    }
}

Deflate::Deflate(unsigned int level)
    : _level(level) {}

inline void Szip::apply(const hid_t hid) const {
    if (!H5Zfilter_avail(H5Z_FILTER_SZIP)) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting szip property");
    }

    if (H5Pset_szip(hid, _options_mask, _pixels_per_block) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting szip property");
    }
}

Szip::Szip(unsigned int options_mask, unsigned int pixels_per_block)
    : _options_mask(options_mask)
    , _pixels_per_block(pixels_per_block) {}

unsigned Szip::getOptionsMask() const {
    return _options_mask;
}

unsigned Szip::getPixelsPerBlock() const {
    return _pixels_per_block;
}

inline void Shuffle::apply(const hid_t hid) const {
    if (!H5Zfilter_avail(H5Z_FILTER_SHUFFLE)) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting shuffle property");
    }

    if (H5Pset_shuffle(hid) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting shuffle property");
    }
}

inline void AllocationTime::apply(hid_t dcpl) const {
    if (H5Pset_alloc_time(dcpl, _alloc_time) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting allocation time");
    }
}

AllocationTime::AllocationTime(H5D_alloc_time_t alloc_time)
    : _alloc_time(alloc_time) {}

inline void Caching::apply(const hid_t hid) const {
    if (H5Pset_chunk_cache(hid, _numSlots, _cacheSize, _w0) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting dataset cache parameters");
    }
}

inline void CreateIntermediateGroup::apply(const hid_t hid) const {
    if (H5Pset_create_intermediate_group(hid, _create ? 1 : 0) < 0) {
        HDF5ErrMapper::ToException<PropertyException>(
            "Error setting property for create intermediate groups");
    }
}

#ifdef H5_HAVE_PARALLEL
inline void UseCollectiveIO::apply(const hid_t hid) const {
    if (H5Pset_dxpl_mpio(hid, _enable ? H5FD_MPIO_COLLECTIVE : H5FD_MPIO_INDEPENDENT) < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting H5Pset_dxpl_mpio.");
    }
}
#endif

}  // namespace HighFive
