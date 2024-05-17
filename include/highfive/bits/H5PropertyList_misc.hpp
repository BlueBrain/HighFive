/*
 *  Copyright (c), 2017-2018, Adrien Devresse <adrien.devresse@epfl.ch>
 *                            Juan Hernando <juan.hernando@epfl.ch>
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include "h5p_wrapper.hpp"

namespace HighFive {

namespace {
inline hid_t convert_plist_type(PropertyType propertyType) {
    // The HP5_XXX are macros with function calls so we can't assign
    // them as the enum values
    switch (propertyType) {
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
    _hid = detail::h5p_create(convert_plist_type(T));
}

template <PropertyType T>
template <PropertyInterface P>
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

inline FileSpaceStrategy::FileSpaceStrategy(const FileCreateProps& fcpl) {
    detail::h5p_get_file_space_strategy(fcpl.getId(), &_strategy, &_persist, &_threshold);
}

inline void FileSpaceStrategy::apply(const hid_t list) const {
    detail::h5p_set_file_space_strategy(list, _strategy, _persist, _threshold);
}

inline H5F_fspace_strategy_t FileSpaceStrategy::getStrategy() const {
    return _strategy;
}

inline hbool_t FileSpaceStrategy::getPersist() const {
    return _persist;
}

inline hsize_t FileSpaceStrategy::getThreshold() const {
    return _threshold;
}

inline FileSpacePageSize::FileSpacePageSize(hsize_t page_size)
    : _page_size(page_size) {}

inline void FileSpacePageSize::apply(const hid_t list) const {
    detail::h5p_set_file_space_page_size(list, _page_size);
}

inline FileSpacePageSize::FileSpacePageSize(const FileCreateProps& fcpl) {
    detail::h5p_get_file_space_page_size(fcpl.getId(), &_page_size);
}

inline hsize_t FileSpacePageSize::getPageSize() const {
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
    detail::h5p_get_page_buffer_size(plist.getId(), &_page_buffer_size, &_min_meta, &_min_raw);
}

inline void PageBufferSize::apply(const hid_t list) const {
    detail::h5p_set_page_buffer_size(list, _page_buffer_size, _min_meta, _min_raw);
}

inline size_t PageBufferSize::getPageBufferSize() const {
    return _page_buffer_size;
}

inline unsigned PageBufferSize::getMinMetaPercent() const {
    return _min_meta;
}

inline unsigned PageBufferSize::getMinRawPercent() const {
    return _min_raw;
}
#endif
#endif

#ifdef H5_HAVE_PARALLEL

inline MPIOFileAccess::MPIOFileAccess(MPI_Comm comm, MPI_Info info)
    : _comm(comm)
    , _info(info) {}

inline void MPIOFileAccess::apply(const hid_t list) const {
    detail::h5p_set_fapl_mpio(list, _comm, _info);
}

#if H5_VERSION_GE(1, 10, 0)
inline void MPIOCollectiveMetadata::apply(const hid_t plist) const {
    auto read = MPIOCollectiveMetadataRead{collective_read_};
    auto write = MPIOCollectiveMetadataWrite{collective_write_};

    read.apply(plist);
    write.apply(plist);
}

inline MPIOCollectiveMetadata::MPIOCollectiveMetadata(bool collective)
    : collective_read_(collective)
    , collective_write_(collective) {}


inline MPIOCollectiveMetadata::MPIOCollectiveMetadata(const FileAccessProps& plist)
    : collective_read_(MPIOCollectiveMetadataRead(plist).isCollective())
    , collective_write_(MPIOCollectiveMetadataWrite(plist).isCollective()) {}

inline bool MPIOCollectiveMetadata::isCollectiveRead() const {
    return collective_read_;
}

inline bool MPIOCollectiveMetadata::isCollectiveWrite() const {
    return collective_write_;
}


inline void MPIOCollectiveMetadataRead::apply(const hid_t plist) const {
    detail::h5p_set_all_coll_metadata_ops(plist, collective_);
}

inline bool MPIOCollectiveMetadataRead::isCollective() const {
    return collective_;
}

inline MPIOCollectiveMetadataRead::MPIOCollectiveMetadataRead(const FileAccessProps& plist) {
    detail::h5p_get_all_coll_metadata_ops(plist.getId(), &collective_);
}

inline MPIOCollectiveMetadataRead::MPIOCollectiveMetadataRead(bool collective)
    : collective_(collective) {}

inline void MPIOCollectiveMetadataWrite::apply(const hid_t plist) const {
    detail::h5p_set_coll_metadata_write(plist, collective_);
}

inline bool MPIOCollectiveMetadataWrite::isCollective() const {
    return collective_;
}

inline MPIOCollectiveMetadataWrite::MPIOCollectiveMetadataWrite(const FileAccessProps& plist) {
    detail::h5p_get_coll_metadata_write(plist.getId(), &collective_);
}

inline MPIOCollectiveMetadataWrite::MPIOCollectiveMetadataWrite(bool collective)
    : collective_(collective) {}

#endif
#endif

inline FileVersionBounds::FileVersionBounds(H5F_libver_t low, H5F_libver_t high)
    : _low(low)
    , _high(high) {}

inline FileVersionBounds::FileVersionBounds(const FileAccessProps& fapl) {
    detail::h5p_get_libver_bounds(fapl.getId(), &_low, &_high);
}

inline std::pair<H5F_libver_t, H5F_libver_t> FileVersionBounds::getVersion() const {
    return std::make_pair(_low, _high);
}

inline void FileVersionBounds::apply(const hid_t list) const {
    detail::h5p_set_libver_bounds(list, _low, _high);
}

inline MetadataBlockSize::MetadataBlockSize(hsize_t size)
    : _size(size) {}

inline MetadataBlockSize::MetadataBlockSize(const FileAccessProps& fapl) {
    detail::h5p_get_meta_block_size(fapl.getId(), &_size);
}

inline void MetadataBlockSize::apply(const hid_t list) const {
    detail::h5p_set_meta_block_size(list, _size);
}

inline hsize_t MetadataBlockSize::getSize() const {
    return _size;
}

inline void EstimatedLinkInfo::apply(const hid_t hid) const {
    detail::h5p_set_est_link_info(hid, _entries, _length);
}

inline EstimatedLinkInfo::EstimatedLinkInfo(unsigned entries, unsigned length)
    : _entries(entries)
    , _length(length) {}

inline EstimatedLinkInfo::EstimatedLinkInfo(const GroupCreateProps& gcpl) {
    detail::h5p_get_est_link_info(gcpl.getId(), &_entries, &_length);
}

inline unsigned EstimatedLinkInfo::getEntries() const {
    return _entries;
}

inline unsigned EstimatedLinkInfo::getNameLength() const {
    return _length;
}

inline void Chunking::apply(const hid_t hid) const {
    detail::h5p_set_chunk(hid, static_cast<int>(_dims.size()), _dims.data());
}

inline Chunking::Chunking(const std::vector<hsize_t>& dims)
    : _dims(dims) {}

inline Chunking::Chunking(const std::initializer_list<hsize_t>& items)
    : Chunking(std::vector<hsize_t>{items}) {}

inline Chunking::Chunking(DataSetCreateProps& plist, size_t max_dims)
    : _dims(max_dims + 1) {
    auto n_loaded =
        detail::h5p_get_chunk(plist.getId(), static_cast<int>(_dims.size()), _dims.data());

    if (n_loaded >= static_cast<int>(_dims.size())) {
        *this = Chunking(plist, 8 * max_dims);
    } else {
        _dims.resize(static_cast<size_t>(n_loaded));
    }
}

inline const std::vector<hsize_t>& Chunking::getDimensions() const noexcept {
    return _dims;
}

template <typename... Args>
inline Chunking::Chunking(hsize_t item, Args... args)
    : Chunking(std::vector<hsize_t>{item, static_cast<hsize_t>(args)...}) {}

inline void Deflate::apply(const hid_t hid) const {
    if (detail::h5z_filter_avail(H5Z_FILTER_DEFLATE) == 0) {
        HDF5ErrMapper::ToException<PropertyException>("Deflate filter unavailable.");
    }

    detail::h5p_set_deflate(hid, _level);
}

inline Deflate::Deflate(unsigned int level)
    : _level(level) {}

inline void Szip::apply(const hid_t hid) const {
    if (detail::h5z_filter_avail(H5Z_FILTER_SZIP) == 0) {
        HDF5ErrMapper::ToException<PropertyException>("SZIP filter unavailable.");
    }

    detail::h5p_set_szip(hid, _options_mask, _pixels_per_block);
}

inline Szip::Szip(unsigned int options_mask, unsigned int pixels_per_block)
    : _options_mask(options_mask)
    , _pixels_per_block(pixels_per_block) {}

inline unsigned Szip::getOptionsMask() const {
    return _options_mask;
}

inline unsigned Szip::getPixelsPerBlock() const {
    return _pixels_per_block;
}

inline void Shuffle::apply(const hid_t hid) const {
    if (detail::h5z_filter_avail(H5Z_FILTER_SHUFFLE) == 0) {
        HDF5ErrMapper::ToException<PropertyException>("Shuffle filter unavailable.");
    }

    detail::h5p_set_shuffle(hid);
}

inline AllocationTime::AllocationTime(H5D_alloc_time_t alloc_time)
    : _alloc_time(alloc_time) {}

inline AllocationTime::AllocationTime(const DataSetCreateProps& dcpl) {
    detail::h5p_get_alloc_time(dcpl.getId(), &_alloc_time);
}

inline void AllocationTime::apply(hid_t dcpl) const {
    detail::h5p_set_alloc_time(dcpl, _alloc_time);
}

inline H5D_alloc_time_t AllocationTime::getAllocationTime() {
    return _alloc_time;
}

inline Caching::Caching(const DataSetCreateProps& dcpl) {
    detail::h5p_get_chunk_cache(dcpl.getId(), &_numSlots, &_cacheSize, &_w0);
}

inline void Caching::apply(const hid_t hid) const {
    detail::h5p_set_chunk_cache(hid, _numSlots, _cacheSize, _w0);
}

inline Caching::Caching(const size_t numSlots, const size_t cacheSize, const double w0)
    : _numSlots(numSlots)
    , _cacheSize(cacheSize)
    , _w0(w0) {}

inline size_t Caching::getNumSlots() const {
    return _numSlots;
}

inline size_t Caching::getCacheSize() const {
    return _cacheSize;
}

inline double Caching::getW0() const {
    return _w0;
}

inline CreateIntermediateGroup::CreateIntermediateGroup(bool create)
    : _create(create) {}

inline void CreateIntermediateGroup::apply(const hid_t hid) const {
    detail::h5p_set_create_intermediate_group(hid, _create ? 1 : 0);
}

inline CreateIntermediateGroup::CreateIntermediateGroup(const LinkCreateProps& lcpl) {
    fromPropertyList(lcpl.getId());
}

inline void CreateIntermediateGroup::fromPropertyList(hid_t hid) {
    unsigned c_bool = 0;
    _create = bool(detail::h5p_get_create_intermediate_group(hid, &c_bool));
}

inline bool CreateIntermediateGroup::isSet() const {
    return _create;
}

#ifdef H5_HAVE_PARALLEL
inline UseCollectiveIO::UseCollectiveIO(bool enable)
    : _enable(enable) {}

inline void UseCollectiveIO::apply(const hid_t hid) const {
    detail::h5p_set_dxpl_mpio(hid, _enable ? H5FD_MPIO_COLLECTIVE : H5FD_MPIO_INDEPENDENT);
}

inline UseCollectiveIO::UseCollectiveIO(const DataTransferProps& dxpl) {
    H5FD_mpio_xfer_t collective;

    detail::h5p_get_dxpl_mpio(dxpl.getId(), &collective);

    if (collective != H5FD_MPIO_COLLECTIVE && collective != H5FD_MPIO_INDEPENDENT) {
        throw std::logic_error("H5Pget_dxpl_mpio returned something strange.");
    }

    _enable = collective == H5FD_MPIO_COLLECTIVE;
}

inline bool UseCollectiveIO::isCollective() const {
    return _enable;
}

inline MpioNoCollectiveCause::MpioNoCollectiveCause(const DataTransferProps& dxpl) {
    detail::h5p_get_mpio_no_collective_cause(dxpl.getId(), &_local_cause, &_global_cause);
}

inline bool MpioNoCollectiveCause::wasCollective() const {
    return _local_cause == 0 && _global_cause == 0;
}

inline uint32_t MpioNoCollectiveCause::getLocalCause() const {
    return _local_cause;
}

inline uint32_t MpioNoCollectiveCause::getGlobalCause() const {
    return _global_cause;
}

inline std::pair<uint32_t, uint32_t> MpioNoCollectiveCause::getCause() const {
    return {_local_cause, _global_cause};
}
#endif

inline LinkCreationOrder::LinkCreationOrder(const FileCreateProps& fcpl) {
    fromPropertyList(fcpl.getId());
}

inline LinkCreationOrder::LinkCreationOrder(const GroupCreateProps& gcpl) {
    fromPropertyList(gcpl.getId());
}

inline unsigned LinkCreationOrder::getFlags() const {
    return _flags;
}

inline void LinkCreationOrder::apply(const hid_t hid) const {
    detail::h5p_set_link_creation_order(hid, _flags);
}

inline void LinkCreationOrder::fromPropertyList(hid_t hid) {
    detail::h5p_get_link_creation_order(hid, &_flags);
}

inline AttributePhaseChange::AttributePhaseChange(unsigned max_compact, unsigned min_dense)
    : _max_compact(max_compact)
    , _min_dense(min_dense) {}

inline AttributePhaseChange::AttributePhaseChange(const GroupCreateProps& gcpl) {
    detail::h5p_get_attr_phase_change(gcpl.getId(), &_max_compact, &_min_dense);
}

inline unsigned AttributePhaseChange::max_compact() const {
    return _max_compact;
}

inline unsigned AttributePhaseChange::min_dense() const {
    return _min_dense;
}

inline void AttributePhaseChange::apply(hid_t hid) const {
    detail::h5p_set_attr_phase_change(hid, _max_compact, _min_dense);
}


}  // namespace HighFive
