/*
 *  Copyright (c), 2017-2018, Adrien Devresse <adrien.devresse@epfl.ch>
 *                            Juan Hernando <juan.hernando@epfl.ch>
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <vector>

#include <H5Ppublic.h>

// Required by MPIOFileAccess
#ifdef H5_HAVE_PARALLEL
#include <H5FDmpi.h>
#endif

#include "H5Exception.hpp"
#include "H5Object.hpp"

namespace HighFive {

/// \defgroup PropertyLists Property Lists
/// HDF5 is configured through what they call property lists. In HDF5 the
/// process has four steps:
///
/// 1. Create a property list. As users we now have an `hid_t` identifying the
/// property list.
/// 2. Set properties as desired.
/// 3. Pass the HID to the HDF5 function to be configured.
/// 4. Free the property list.
///
/// Note that the mental picture is that one creates a settings object, and
/// then passes those settings to a function such as `H5Dwrite`. In and of
/// themselves the settings don't change the behaviour of HDF5. Rather they
/// need to be used to take affect.
///
/// The second aspect is that property lists represent any number of related
/// settings, e.g. there's property lists anything related to creating files
/// and another for accessing files, same for creating and accessing datasets,
/// etc. Settings that affect creating files, must be passed a file creation
/// property list, while settings that affect file access require a file access
/// property list.
///
/// In HighFive the `PropertyList` works similar in that it's a object
/// representing the settings, i.e. internally it's just the property lists
/// HID. Just like in HDF5 one adds the settings to the settings object; and
/// then passes the settings object to the respective method. Example:
///
///
///     // Create an object which contains the setting to
///     // open files with MPI-IO.
///     auto fapl = FileAccessProps();
///     fapl.add(MPIOFileAccess(MPI_COMM_WORLD, MPI_INFO_NULL);
///
///     // To open a specific file with MPI-IO, we do:
///     auto file = File("foo.h5", File::ReadOnly, fapl);
///
/// Note that the `MPIOFileAccess` object by itself doesn't affect the
/// `FileAccessProps`. Rather it needs to be explicitly added to the `fapl`
/// (the group of file access related settings), and then the `fapl` needs to
/// be passed to the constructor of `File` for the settings to take affect.
///
/// This is important to understand when reading properties. Example:
///
///     // Obtain the file access property list:
///     auto fapl = file.getAccessPropertyList()
///
///     // Extracts a copy of the collective MPI-IO metadata settings from
///     // the group of file access related setting, i.e. the `fapl`:
///     auto mpio_metadata = MPIOCollectiveMetadata(fapl);
///
///     if(mpio_metadata.isCollectiveRead()) {
///       // something specific if meta data is read collectively.
///     }
///
///     // Careful, this only affects the `mpio_metadata` object, but not the
///     //  `fapl`, and also not whether `file` uses collective MPI-IO for
///     // metadata.
///     mpio_metadata = MPIOCollectiveMetadata(false, false);
///
/// @{

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

namespace details {
template <typename T, typename U>
T get_plist(const U& obj, hid_t (*f)(hid_t)) {
    auto hid = f(obj.getId());
    if (hid < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Unable to get property list");
    }
    T t{};
    t._hid = hid;
    return t;
}
}  // namespace details

///
/// \brief Base Class for Property lists, providing global default
class PropertyListBase: public Object {
  public:
    PropertyListBase() noexcept;

    static const PropertyListBase& Default() noexcept {
        static const PropertyListBase plist{};
        return plist;
    }

  private:
    template <typename T, typename U>
    friend T details::get_plist(const U&, hid_t (*f)(hid_t));
};

/// \interface PropertyInterface
/// \brief HDF5 file property object
///
/// A property is an object which is expected to have a method with the
/// following signature `void apply(hid_t hid) const`
///
/// \sa Instructions to document C++20 concepts with Doxygen: https://github.com/doxygen/doxygen/issues/2732#issuecomment-509629967
///
/// \cond
#if HIGHFIVE_HAS_CONCEPTS && __cplusplus >= 202002L
template <typename P>
concept PropertyInterface = requires(P p, const hid_t hid) {
    {p.apply(hid)};
};

#else
#define PropertyInterface typename
#endif
/// \endcond

///
/// \brief HDF5 property Lists
///
template <PropertyType T>
class PropertyList: public PropertyListBase {
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
    /// \tparam PropertyInterface
    template <PropertyInterface P>
    void add(const P& property);

    ///
    /// Return the Default property type object
    static const PropertyList<T>& Default() noexcept {
        return static_cast<const PropertyList<T>&>(PropertyListBase::Default());
    }

    /// Return a property list created via a call to `H5Pcreate`.
    ///
    /// An empty property is needed when one wants `getId()` to immediately
    /// point at a valid HID. This is important when interfacing directly with
    /// HDF5 to set properties that haven't been wrapped by HighFive.
    static PropertyList<T> Empty() {
        auto plist = PropertyList<T>();
        plist._initializeIfNeeded();

        return plist;
    }

  protected:
    void _initializeIfNeeded();
};

using ObjectCreateProps = PropertyList<PropertyType::OBJECT_CREATE>;
using FileCreateProps = PropertyList<PropertyType::FILE_CREATE>;
using FileAccessProps = PropertyList<PropertyType::FILE_ACCESS>;
using DataSetCreateProps = PropertyList<PropertyType::DATASET_CREATE>;
using DataSetAccessProps = PropertyList<PropertyType::DATASET_ACCESS>;
using DataTransferProps = PropertyList<PropertyType::DATASET_XFER>;
using GroupCreateProps = PropertyList<PropertyType::GROUP_CREATE>;
using GroupAccessProps = PropertyList<PropertyType::GROUP_ACCESS>;
using DataTypeCreateProps = PropertyList<PropertyType::DATATYPE_CREATE>;
using DataTypeAccessProps = PropertyList<PropertyType::DATATYPE_ACCESS>;
using StringCreateProps = PropertyList<PropertyType::STRING_CREATE>;
using AttributeCreateProps = PropertyList<PropertyType::ATTRIBUTE_CREATE>;
using ObjectCopyProps = PropertyList<PropertyType::OBJECT_COPY>;
using LinkCreateProps = PropertyList<PropertyType::LINK_CREATE>;
using LinkAccessProps = PropertyList<PropertyType::LINK_ACCESS>;

///
/// RawPropertyLists are to be used when advanced H5 properties
/// are desired and are not part of the HighFive API.
/// Therefore this class is mainly for internal use.
template <PropertyType T>
class RawPropertyList: public PropertyList<T> {
  public:
    template <typename F, typename... Args>
    void add(const F& funct, const Args&... args);
};

#ifdef H5_HAVE_PARALLEL
///
/// \brief Configure MPI access for the file
///
/// All further modifications to the structure of the file will have to be
/// done with collective operations
///
class MPIOFileAccess {
  public:
    MPIOFileAccess(MPI_Comm comm, MPI_Info info);

  private:
    friend FileAccessProps;
    void apply(const hid_t list) const;

    MPI_Comm _comm;
    MPI_Info _info;
};


#if H5_VERSION_GE(1, 10, 0)
///
/// \brief Use collective MPI-IO for metadata read and write.
///
/// See `MPIOCollectiveMetadataRead` and `MPIOCollectiveMetadataWrite`.
///
class MPIOCollectiveMetadata {
  public:
    explicit MPIOCollectiveMetadata(bool collective = true);
    explicit MPIOCollectiveMetadata(const FileAccessProps& plist);

    bool isCollectiveRead() const;
    bool isCollectiveWrite() const;


  private:
    friend FileAccessProps;
    void apply(hid_t plist) const;

    bool collective_read_;
    bool collective_write_;
};

///
/// \brief Use collective MPI-IO for metadata read?
///
/// Note that when used in a file access property list, this will force all reads
/// of meta data to be collective. HDF5 function may implicitly perform metadata
/// reads. These functions would become collective. A list of functions that
/// perform metadata reads can be found in the HDF5 documentation, e.g.
///    https://docs.hdfgroup.org/hdf5/v1_12/group___g_a_c_p_l.html
///
/// In HighFive setting collective read is (currently) only supported on file level.
///
/// Please also consult upstream documentation of `H5Pset_all_coll_metadata_ops`.
///
class MPIOCollectiveMetadataRead {
  public:
    explicit MPIOCollectiveMetadataRead(bool collective = true);
    explicit MPIOCollectiveMetadataRead(const FileAccessProps& plist);

    bool isCollective() const;

  private:
    friend FileAccessProps;
    friend MPIOCollectiveMetadata;

    void apply(hid_t plist) const;

    bool collective_;
};

///
/// \brief Use collective MPI-IO for metadata write?
///
/// In order to keep the in-memory representation of the file structure
/// consistent across MPI ranks, writing meta data is always a collective
/// operation. Meaning all MPI ranks must participate. Passing this setting
/// enables using MPI-IO collective operations for metadata writes.
///
/// Please also consult upstream documentation of `H5Pset_coll_metadata_write`.
///
class MPIOCollectiveMetadataWrite {
  public:
    explicit MPIOCollectiveMetadataWrite(bool collective = true);
    explicit MPIOCollectiveMetadataWrite(const FileAccessProps& plist);

    bool isCollective() const;

  private:
    friend FileAccessProps;
    friend MPIOCollectiveMetadata;

    void apply(hid_t plist) const;

    bool collective_;
};

#endif
#endif

///
/// \brief Configure the version bounds for the file
///
/// Used to define the compatibility of objects created within HDF5 files,
/// and affects the format of groups stored in the file.
///
/// See also the documentation of \c H5P_SET_LIBVER_BOUNDS in HDF5.
///
/// Possible values for \c low and \c high are:
/// * \c H5F_LIBVER_EARLIEST
/// * \c H5F_LIBVER_V18
/// * \c H5F_LIBVER_V110
/// * \c H5F_LIBVER_NBOUNDS
/// * \c H5F_LIBVER_LATEST currently defined as \c H5F_LIBVER_V110 within
///   HDF5
///
class FileVersionBounds {
  public:
    FileVersionBounds(H5F_libver_t low, H5F_libver_t high);
    explicit FileVersionBounds(const FileAccessProps& fapl);

    std::pair<H5F_libver_t, H5F_libver_t> getVersion() const;

  private:
    friend FileAccessProps;
    void apply(const hid_t list) const;

    H5F_libver_t _low;
    H5F_libver_t _high;
};

///
/// \brief Configure the metadata block size to use writing to files
///
/// \param size Metadata block size in bytes
///
class MetadataBlockSize {
  public:
    explicit MetadataBlockSize(hsize_t size);
    explicit MetadataBlockSize(const FileAccessProps& fapl);

    hsize_t getSize() const;

  private:
    friend FileAccessProps;
    void apply(const hid_t list) const;
    hsize_t _size;
};

#if H5_VERSION_GE(1, 10, 1)
///
/// \brief Configure the file space strategy.
///
/// See the upstream documentation of `H5Pget_file_space_strategy` for more details. Essentially,
/// it enables configuring how space is allocate in the file.
///
class FileSpaceStrategy {
  public:
    ///
    /// \brief Create a file space strategy property.
    ///
    /// \param strategy The HDF5 free space strategy.
    /// \param persist Should free space managers be persisted across file closing and reopening.
    /// \param threshold The free-space manager wont track sections small than this threshold.
    FileSpaceStrategy(H5F_fspace_strategy_t strategy, hbool_t persist, hsize_t threshold);
    explicit FileSpaceStrategy(const FileCreateProps& fcpl);

    H5F_fspace_strategy_t getStrategy() const;
    hbool_t getPersist() const;
    hsize_t getThreshold() const;

  private:
    friend FileCreateProps;

    void apply(const hid_t list) const;

    H5F_fspace_strategy_t _strategy;
    hbool_t _persist;
    hsize_t _threshold;
};

///
/// \brief Configure the page size for paged allocation.
///
/// See the upstream documentation of `H5Pset_file_space_page_size` for more details. Essentially,
/// it enables configuring the page size when paged allocation is used.
///
/// General information about paged allocation can be found in the upstream documentation "RFC: Page
/// Buffering".
///
class FileSpacePageSize {
  public:
    ///
    /// \brief Create a file space strategy property.
    ///
    /// \param page_size The page size in bytes.
    explicit FileSpacePageSize(hsize_t page_size);
    explicit FileSpacePageSize(const FileCreateProps& fcpl);

    hsize_t getPageSize() const;

  private:
    friend FileCreateProps;
    void apply(const hid_t list) const;

    hsize_t _page_size;
};

#ifndef H5_HAVE_PARALLEL
/// \brief Set size of the page buffer.
///
/// Please, consult the upstream documentation of
///    H5Pset_page_buffer_size
///    H5Pget_page_buffer_size
/// Note that this setting is only valid for page allocated/aggregated
/// files, i.e. those that have file space strategy "Page".
///
/// Tests suggest this doesn't work in the parallel version of the
/// library. Hence, this isn't available at compile time if the parallel
/// library was selected.
class PageBufferSize {
  public:
    /// Property to set page buffer sizes.
    ///
    /// @param page_buffer_size maximum size of the page buffer in bytes.
    /// @param min_meta_percent fraction of the page buffer dedicated to meta data, in percent.
    /// @param min_raw_percent fraction of the page buffer dedicated to raw data, in percent.
    explicit PageBufferSize(size_t page_buffer_size,
                            unsigned min_meta_percent = 0,
                            unsigned min_raw_percent = 0);

    explicit PageBufferSize(const FileAccessProps& fapl);

    size_t getPageBufferSize() const;
    unsigned getMinMetaPercent() const;
    unsigned getMinRawPercent() const;

  private:
    friend FileAccessProps;

    void apply(hid_t list) const;

    size_t _page_buffer_size;
    unsigned _min_meta;
    unsigned _min_raw;
};
#endif
#endif

/// \brief Set hints as to how many links to expect and their average length
/// \implements PropertyInterface
///
class EstimatedLinkInfo {
  public:
    /// \brief Create a property with the request parameters.
    ///
    /// @param entries The estimated number of links in a group.
    /// @param length The estimated length of the names of links.
    explicit EstimatedLinkInfo(unsigned entries, unsigned length);

    explicit EstimatedLinkInfo(const GroupCreateProps& gcpl);

    /// \brief The estimated number of links in a group.
    unsigned getEntries() const;

    /// \brief The estimated length of the names of links.
    unsigned getNameLength() const;

  private:
    friend GroupCreateProps;
    void apply(hid_t hid) const;
    unsigned _entries;
    unsigned _length;
};


/// \implements PropertyInterface
class Chunking {
  public:
    explicit Chunking(const std::vector<hsize_t>& dims);
    Chunking(const std::initializer_list<hsize_t>& items);

    template <typename... Args>
    explicit Chunking(hsize_t item, Args... args);

    explicit Chunking(DataSetCreateProps& plist, size_t max_dims = 32);

    const std::vector<hsize_t>& getDimensions() const noexcept;

  private:
    friend DataSetCreateProps;
    void apply(hid_t hid) const;
    std::vector<hsize_t> _dims;
};

/// \implements PropertyInterface
class Deflate {
  public:
    explicit Deflate(unsigned level);

  private:
    friend DataSetCreateProps;
    friend GroupCreateProps;
    void apply(hid_t hid) const;
    const unsigned _level;
};

/// \implements PropertyInterface
class Szip {
  public:
    explicit Szip(unsigned options_mask = H5_SZIP_EC_OPTION_MASK,
                  unsigned pixels_per_block = H5_SZIP_MAX_PIXELS_PER_BLOCK);

    unsigned getOptionsMask() const;
    unsigned getPixelsPerBlock() const;

  private:
    friend DataSetCreateProps;
    void apply(hid_t hid) const;
    const unsigned _options_mask;
    const unsigned _pixels_per_block;
};

/// \implements PropertyInterface
class Shuffle {
  public:
    Shuffle() = default;

  private:
    friend DataSetCreateProps;
    void apply(hid_t hid) const;
};

/// \brief When are datasets allocated?
///
/// The precise time of when HDF5 requests space to store the dataset
/// can be configured. Please, consider the upstream documentation for
/// `H5Pset_alloc_time`.
/// \implements PropertyInterface
class AllocationTime {
  public:
    explicit AllocationTime(H5D_alloc_time_t alloc_time);
    explicit AllocationTime(const DataSetCreateProps& dcpl);

    H5D_alloc_time_t getAllocationTime();

  private:
    friend DataSetCreateProps;
    void apply(hid_t dcpl) const;

    H5D_alloc_time_t _alloc_time;
};

/// Dataset access property to control chunk cache configuration.
/// Do not confuse with the similar file access property for H5Pset_cache
/// \implements PropertyInterface
class Caching {
  public:
    /// https://support.hdfgroup.org/HDF5/doc/RM/H5P/H5Pset_chunk_cache.html for
    /// details.
    Caching(const size_t numSlots,
            const size_t cacheSize,
            const double w0 = static_cast<double>(H5D_CHUNK_CACHE_W0_DEFAULT));

    explicit Caching(const DataSetCreateProps& dcpl);

    size_t getNumSlots() const;
    size_t getCacheSize() const;
    double getW0() const;

  private:
    friend DataSetAccessProps;
    void apply(hid_t hid) const;
    size_t _numSlots;
    size_t _cacheSize;
    double _w0;
};

/// \implements PropertyInterface
class CreateIntermediateGroup {
  public:
    explicit CreateIntermediateGroup(bool create = true);

    explicit CreateIntermediateGroup(const ObjectCreateProps& ocpl);
    explicit CreateIntermediateGroup(const LinkCreateProps& lcpl);

    bool isSet() const;

  protected:
    void fromPropertyList(hid_t hid);

  private:
    friend ObjectCreateProps;
    friend LinkCreateProps;
    void apply(hid_t hid) const;
    bool _create;
};

#ifdef H5_HAVE_PARALLEL
/// \implements PropertyInterface
class UseCollectiveIO {
  public:
    explicit UseCollectiveIO(bool enable = true);

    explicit UseCollectiveIO(const DataTransferProps& dxpl);

    /// \brief Does the property request collective IO?
    bool isCollective() const;

  private:
    friend DataTransferProps;
    void apply(hid_t hid) const;
    bool _enable;
};


/// \brief The cause for non-collective I/O.
///
/// The cause refers to the most recent I/O with data transfer property list  `dxpl` at time of
/// creation of this object. This object will not update automatically for later data transfers,
/// i.e. `H5Pget_mpio_no_collective_cause` is called in the constructor, and not when fetching
/// a value, such as `wasCollective`.
/// \implements PropertyInterface
class MpioNoCollectiveCause {
  public:
    explicit MpioNoCollectiveCause(const DataTransferProps& dxpl);

    /// \brief Was the datatransfer collective?
    bool wasCollective() const;

    /// \brief The local cause for a non-collective I/O.
    uint32_t getLocalCause() const;

    /// \brief The global cause for a non-collective I/O.
    uint32_t getGlobalCause() const;

    /// \brief A pair of the local and global cause for non-collective I/O.
    std::pair<uint32_t, uint32_t> getCause() const;

  private:
    friend DataTransferProps;
    uint32_t _local_cause;
    uint32_t _global_cause;
};
#endif

struct CreationOrder {
    enum _CreationOrder {
        Tracked = H5P_CRT_ORDER_TRACKED,
        Indexed = H5P_CRT_ORDER_INDEXED,
    };
};

///
/// \brief Track and index creation order time
///
/// Let user retrieve objects by creation order time instead of name.
///
/// \implements PropertyInterface
class LinkCreationOrder {
  public:
    ///
    /// \brief Create the property
    /// \param flags Should be a composition of HighFive::CreationOrder.
    ///
    explicit LinkCreationOrder(unsigned flags)
        : _flags(flags) {}

    explicit LinkCreationOrder(const FileCreateProps& fcpl);
    explicit LinkCreationOrder(const GroupCreateProps& gcpl);

    unsigned getFlags() const;

  protected:
    void fromPropertyList(hid_t hid);

  private:
    friend FileCreateProps;
    friend GroupCreateProps;
    void apply(hid_t hid) const;
    unsigned _flags;
};


///
/// \brief Set threshold for attribute storage.
///
/// HDF5 can store Attributes in the object header (compact) or in the B-tree
/// (dense). This property sets the threshold when attributes are moved to one
/// or the other storage format.
///
/// Please refer to the upstream documentation of `H5Pset_attr_phase_change` or
/// Section 8 (Attributes) in the User Guide, in particular Subsection 8.5.
///
/// \implements PropertyInterface
class AttributePhaseChange {
  public:
    ///
    /// \brief Create the property from the threshold values.
    ///
    /// When the number of attributes hits `max_compact` the attributes are
    /// moved to dense storage, once the number drops to below `min_dense` the
    /// attributes are moved to compact storage.
    AttributePhaseChange(unsigned max_compact, unsigned min_dense);

    /// \brief Extract threshold values from property list.
    explicit AttributePhaseChange(const GroupCreateProps& gcpl);

    unsigned max_compact() const;
    unsigned min_dense() const;

  private:
    friend GroupCreateProps;
    void apply(hid_t hid) const;

    unsigned _max_compact;
    unsigned _min_dense;
};

/// @}

}  // namespace HighFive

#include "bits/H5PropertyList_misc.hpp"
