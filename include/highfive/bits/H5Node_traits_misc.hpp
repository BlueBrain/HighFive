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
#include <algorithm>
#include <vector>
#include <functional>
#include <cmath>

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

static inline std::vector<hsize_t> _guessChunkDims(const std::vector<size_t>& dims,
                                                   const std::vector<size_t>& max_dims,
                                                   size_t typesize) {
    // Based on the h5py implementation
    const size_t CHUNK_BASE = 16 * 1024;   // Multiplier by which chunks are adjusted
    const size_t CHUNK_MIN = 8 * 1024;     // Soft lower limit (8k)
    const size_t CHUNK_MAX = 1024 * 1024;  // Hard upper limit (1M)
    auto product = [](std::vector<size_t> vec) {
        return std::accumulate(vec.begin(), vec.end(), size_t(1), std::multiplies<size_t>());
    };

    size_t ndims = dims.size();
    if (ndims == 0) {
        HDF5ErrMapper::ToException<DataSetException>(
            std::string("Chunks not allowed for scalar datasets."));
    }
    std::vector<size_t> chunkdims = dims;

    // If the dimension is unlimited, set chunksize to 1024 along that
    for (int i = 0; i < ndims; i++) {
        if (max_dims[i] == DataSpace::UNLIMITED)
            chunkdims[i] = 1024;
    }

    size_t chunk_size;
    size_t dset_size = product(chunkdims) * typesize;
    double target_size = CHUNK_BASE * std::pow(2.0, std::log10(dset_size / (1024. * 1024)));

    if (target_size > CHUNK_MAX)
        target_size = CHUNK_MAX;
    else if (target_size < CHUNK_MIN)
        target_size = CHUNK_MIN;

    int idx = 0;
    while (1) {
        // Repeatedly loop over the axes, dividing them by 2.  Stop when:
        // 1a. We're smaller than the target chunk size, OR
        // 1b. We're within 50% of the target chunk size, AND
        //  2. The chunk is smaller than the maximum chunk size

        chunk_size = product(chunkdims) * typesize;

        if ((chunk_size < target_size || std::abs(chunk_size - target_size) / target_size < 0.5) &&
            chunk_size < CHUNK_MAX)
            break;

        if (product(chunkdims) == 1)
            break;  // Element size larger than CHUNK_MAX

        chunkdims[idx % ndims] = static_cast<size_t>(std::ceil(chunkdims[idx % ndims] / 2.0));
        idx++;
    }

    return details::to_vector_hsize_t(chunkdims);
}

template <typename Derivate>
inline DataSetCreateProps NodeTraits<Derivate>::_chunkIfNecessary(
    const std::string& dataset_name,
    const DataSpace& space,
    const DataType& dtype,
    const DataSetCreateProps& createProps) {
    bool shuffleSet, deflateSet, szipSet, chunkedLayout;

    // Check whether the dataset layout is chunked or not
    if (createProps.getId() == H5P_DEFAULT) {
        chunkedLayout = false;  // Default dataset layout is contiguous
    } else {
        H5D_layout_t layout = H5Pget_layout(createProps.getId());
        if (layout < 0) {
            HDF5ErrMapper::ToException<DataSetException>(
                std::string("Unable to query the layout for dataset \"") + dataset_name + "\":");
        }
        chunkedLayout = (layout == H5D_CHUNKED);
    }

    // Query options which require chunked layout
    {
        SilenceHDF5 silencer;
        const hid_t& hid = createProps._hid;
        unsigned int flags;
        shuffleSet =
            H5Pget_filter_by_id(hid, H5Z_FILTER_SHUFFLE, &flags, NULL, NULL, 0, NULL, NULL) >= 0;
        deflateSet =
            H5Pget_filter_by_id(hid, H5Z_FILTER_DEFLATE, &flags, NULL, NULL, 0, NULL, NULL) >= 0;
        szipSet = H5Pget_filter_by_id(hid, H5Z_FILTER_SZIP, &flags, NULL, NULL, 0, NULL, NULL) >= 0;
    }
    bool extendable = !std::equal(space.getDimensions().begin(),
                                  space.getDimensions().end(),
                                  space.getMaxDimensions().begin());

    // If layout is not chunked but necessary, guess chunk size
    if ((extendable || shuffleSet || deflateSet || szipSet) && !chunkedLayout) {
        DataSetCreateProps createPropsNew;
        createPropsNew._hid = H5Pcopy(createProps.getId());
        std::vector<hsize_t> chunkDims;
        chunkDims =
            _guessChunkDims(space.getDimensions(), space.getMaxDimensions(), dtype.getSize());
        createPropsNew.add(Chunking(chunkDims));
        return createPropsNew;
    }
    return createProps;
}

template <typename Derivate>
inline DataSet NodeTraits<Derivate>::createDataSet(const std::string& dataset_name,
                                                   const DataSpace& space,
                                                   const DataType& dtype,
                                                   const DataSetCreateProps& createProps,
                                                   const DataSetAccessProps& accessProps,
                                                   bool parents) {
    LinkCreateProps lcpl;
    lcpl.add(CreateIntermediateGroup(parents));
    DataSetCreateProps finalCreateProps;
    finalCreateProps = _chunkIfNecessary(dataset_name, space, dtype, createProps);
    const auto hid = H5Dcreate2(static_cast<Derivate*>(this)->getId(),
                                dataset_name.c_str(),
                                dtype._hid,
                                space._hid,
                                lcpl.getId(),
                                finalCreateProps.getId(),
                                accessProps.getId());
    if (hid < 0) {
        HDF5ErrMapper::ToException<DataSetException>(
            std::string("Unable to create the dataset \"") + dataset_name + "\":");
    }
    return DataSet(hid);
}

template <typename Derivate>
template <typename T,
          typename std::enable_if<
              std::is_same<typename details::inspector<T>::base_type, details::Boolean>::value,
              int>::type*>
inline DataSet NodeTraits<Derivate>::createDataSet(const std::string& dataset_name,
                                                   const DataSpace& space,
                                                   const DataSetCreateProps& createProps,
                                                   const DataSetAccessProps& accessProps,
                                                   bool parents) {
    return createDataSet(dataset_name,
                         space,
                         create_and_check_datatype<typename details::inspector<T>::base_type>(),
                         createProps,
                         accessProps,
                         parents);
}

template <typename Derivate>
template <typename T,
          typename std::enable_if<
              !std::is_same<typename details::inspector<T>::base_type, details::Boolean>::value,
              int>::type*>
inline DataSet NodeTraits<Derivate>::createDataSet(const std::string& dataset_name,
                                                   const DataSpace& space,
                                                   const DataSetCreateProps& createProps,
                                                   const DataSetAccessProps& accessProps,
                                                   bool parents) {
    return createDataSet(
        dataset_name, space, create_and_check_datatype<T>(), createProps, accessProps, parents);
}

template <typename Derivate>
template <typename T>
inline DataSet NodeTraits<Derivate>::createDataSet(const std::string& dataset_name,
                                                   const T& data,
                                                   const DataSetCreateProps& createProps,
                                                   const DataSetAccessProps& accessProps,
                                                   bool parents) {
    DataSet ds =
        createDataSet(dataset_name,
                      DataSpace::From(data),
                      create_and_check_datatype<typename details::inspector<T>::base_type>(),
                      createProps,
                      accessProps,
                      parents);
    ds.write(data);
    return ds;
}

template <typename Derivate>
template <std::size_t N>
inline DataSet NodeTraits<Derivate>::createDataSet(const std::string& dataset_name,
                                                   const FixedLenStringArray<N>& data,
                                                   const DataSetCreateProps& createProps,
                                                   const DataSetAccessProps& accessProps,
                                                   bool parents) {
    DataSet ds = createDataSet<char[N]>(
        dataset_name, DataSpace(data.size()), createProps, accessProps, parents);
    ds.write(data);
    return ds;
}

template <typename Derivate>
inline DataSet NodeTraits<Derivate>::getDataSet(const std::string& dataset_name,
                                                const DataSetAccessProps& accessProps) const {
    const auto hid = H5Dopen2(static_cast<const Derivate*>(this)->getId(),
                              dataset_name.c_str(),
                              accessProps.getId());
    if (hid < 0) {
        HDF5ErrMapper::ToException<DataSetException>(std::string("Unable to open the dataset \"") +
                                                     dataset_name + "\":");
    }
    return DataSet(hid);
}

template <typename Derivate>
inline Group NodeTraits<Derivate>::createGroup(const std::string& group_name, bool parents) {
    LinkCreateProps lcpl;
    lcpl.add(CreateIntermediateGroup(parents));
    const auto hid = H5Gcreate2(static_cast<Derivate*>(this)->getId(),
                                group_name.c_str(),
                                lcpl.getId(),
                                H5P_DEFAULT,
                                H5P_DEFAULT);
    if (hid < 0) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to create the group \"") +
                                                   group_name + "\":");
    }
    return Group(hid);
}

template <typename Derivate>
inline Group NodeTraits<Derivate>::createGroup(const std::string& group_name,
                                               const GroupCreateProps& createProps,
                                               bool parents) {
    LinkCreateProps lcpl;
    lcpl.add(CreateIntermediateGroup(parents));
    const auto hid = H5Gcreate2(static_cast<Derivate*>(this)->getId(),
                                group_name.c_str(),
                                lcpl.getId(),
                                createProps.getId(),
                                H5P_DEFAULT);
    if (hid < 0) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to create the group \"") +
                                                   group_name + "\":");
    }
    return Group(hid);
}

template <typename Derivate>
inline Group NodeTraits<Derivate>::getGroup(const std::string& group_name) const {
    const auto hid =
        H5Gopen2(static_cast<const Derivate*>(this)->getId(), group_name.c_str(), H5P_DEFAULT);
    if (hid < 0) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to open the group \"") +
                                                   group_name + "\":");
    }
    return Group(hid);
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
    return details::get_name([&](char* buffer, size_t length) {
        return H5Lget_name_by_idx(static_cast<const Derivate*>(this)->getId(),
                                  ".",
                                  H5_INDEX_NAME,
                                  H5_ITER_INC,
                                  index,
                                  buffer,
                                  length,
                                  H5P_DEFAULT);
    });
}

template <typename Derivate>
inline bool NodeTraits<Derivate>::rename(const std::string& src_path,
                                         const std::string& dst_path,
                                         bool parents) const {
    LinkCreateProps lcpl;
    lcpl.add(CreateIntermediateGroup(parents));
    herr_t status = H5Lmove(static_cast<const Derivate*>(this)->getId(),
                            src_path.c_str(),
                            static_cast<const Derivate*>(this)->getId(),
                            dst_path.c_str(),
                            lcpl.getId(),
                            H5P_DEFAULT);
    if (status < 0) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to move link to \"") +
                                                   dst_path + "\":");
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

    if (H5Literate(static_cast<const Derivate*>(this)->getId(),
                   H5_INDEX_NAME,
                   H5_ITER_INC,
                   NULL,
                   &details::internal_high_five_iterate<H5L_info_t>,
                   static_cast<void*>(&iterateData)) < 0) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to list objects in group"));
    }

    return names;
}

template <typename Derivate>
inline bool NodeTraits<Derivate>::_exist(const std::string& node_name, bool raise_errors) const {
    SilenceHDF5 silencer{};
    const auto val =
        H5Lexists(static_cast<const Derivate*>(this)->getId(), node_name.c_str(), H5P_DEFAULT);
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
        // Unless "/" (already checked), verify path exists (not throwing errors)
        return (group_path == "/") ? true : _exist(group_path, false);
    }
    return _exist(group_path);
}


template <typename Derivate>
inline void NodeTraits<Derivate>::unlink(const std::string& node_name) const {
    const herr_t val =
        H5Ldelete(static_cast<const Derivate*>(this)->getId(), node_name.c_str(), H5P_DEFAULT);
    if (val < 0) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Invalid name for unlink() "));
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
                    node_name.c_str(),
                    &linkinfo,
                    H5P_DEFAULT) < 0 ||
        linkinfo.type == H5L_TYPE_ERROR) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to obtain info for link ") +
                                                   node_name);
    }
    return _convert_link_type(linkinfo.type);
}

template <typename Derivate>
inline ObjectType NodeTraits<Derivate>::getObjectType(const std::string& node_name) const {
    return _open(node_name).getType();
}


template <typename Derivate>
inline void NodeTraits<Derivate>::createSoftLink(const std::string& link_name,
                                                 const std::string& obj_path,
                                                 LinkCreateProps linkCreateProps,
                                                 const LinkAccessProps& linkAccessProps,
                                                 const bool parents) {
    if (parents) {
        linkCreateProps.add(CreateIntermediateGroup{});
    }
    auto status = H5Lcreate_soft(obj_path.c_str(),
                                 static_cast<const Derivate*>(this)->getId(),
                                 link_name.c_str(),
                                 linkCreateProps.getId(),
                                 linkAccessProps.getId());
    if (status < 0) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to create soft link: "));
    }
}


template <typename Derivate>
inline void NodeTraits<Derivate>::createExternalLink(const std::string& link_name,
                                                     const std::string& h5_file,
                                                     const std::string& obj_path,
                                                     LinkCreateProps linkCreateProps,
                                                     const LinkAccessProps& linkAccessProps,
                                                     const bool parents) {
    if (parents) {
        linkCreateProps.add(CreateIntermediateGroup{});
    }
    auto status = H5Lcreate_external(h5_file.c_str(),
                                     obj_path.c_str(),
                                     static_cast<const Derivate*>(this)->getId(),
                                     link_name.c_str(),
                                     linkCreateProps.getId(),
                                     linkAccessProps.getId());
    if (status < 0) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to create external link: "));
    }
}


template <typename Derivate>
inline Object NodeTraits<Derivate>::_open(const std::string& node_name,
                                          const DataSetAccessProps& accessProps) const {
    const auto id = H5Oopen(static_cast<const Derivate*>(this)->getId(),
                            node_name.c_str(),
                            accessProps.getId());
    if (id < 0) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to open \"") + node_name +
                                                   "\":");
    }
    return Object(id);
}


}  // namespace HighFive
