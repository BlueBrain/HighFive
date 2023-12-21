#pragma once

#include <H5Ipublic.h>
#include <H5Ppublic.h>

namespace HighFive {
namespace detail {
inline hid_t h5p_create(hid_t cls_id) {
    hid_t plist_id = H5Pcreate(cls_id);
    if (plist_id == H5I_INVALID_HID) {
        HDF5ErrMapper::ToException<PropertyException>("Failed to create property list");
    }

    return plist_id;
}

#if H5_VERSION_GE(1, 10, 1)
inline herr_t h5p_set_file_space_strategy(hid_t plist_id,
                                          H5F_fspace_strategy_t strategy,
                                          hbool_t persist,
                                          hsize_t threshold) {
    herr_t err = H5Pset_file_space_strategy(plist_id, strategy, persist, threshold);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Unable to get file space strategy");
    }

    return err;
}

inline herr_t h5p_get_file_space_strategy(hid_t plist_id,
                                          H5F_fspace_strategy_t* strategy,
                                          hbool_t* persist,
                                          hsize_t* threshold) {
    herr_t err = H5Pget_file_space_strategy(plist_id, strategy, persist, threshold);
    if (err) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting file space strategy.");
    }

    return err;
}

inline herr_t h5p_set_file_space_page_size(hid_t plist_id, hsize_t fsp_size) {
    herr_t err = H5Pset_file_space_page_size(plist_id, fsp_size);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting file space page size.");
    }

    return err;
}

inline herr_t h5p_get_file_space_page_size(hid_t plist_id, hsize_t* fsp_size) {
    herr_t err = H5Pget_file_space_page_size(plist_id, fsp_size);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Unable to get file space page size");
    }

    return err;
}

#ifndef H5_HAVE_PARALLEL
inline herr_t h5p_get_page_buffer_size(hid_t plist_id,
                                       size_t* buf_size,
                                       unsigned* min_meta_perc,
                                       unsigned* min_raw_perc) {
    herr_t err = H5Pget_page_buffer_size(plist_id, buf_size, min_meta_perc, min_raw_perc);

    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting page buffer size.");
    }

    return err;
}

inline herr_t h5p_set_page_buffer_size(hid_t plist_id,
                                       size_t buf_size,
                                       unsigned min_meta_per,
                                       unsigned min_raw_per) {
    herr_t err = H5Pset_page_buffer_size(plist_id, buf_size, min_meta_per, min_raw_per);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting page buffer size.");
    }

    return err;
}
#endif
#endif

#ifdef H5_HAVE_PARALLEL
inline herr_t h5p_set_fapl_mpio(hid_t fapl_id, MPI_Comm comm, MPI_Info info) {
    herr_t err = H5Pset_fapl_mpio(fapl_id, comm, info);
    if (err < 0) {
        HDF5ErrMapper::ToException<FileException>("Unable to set-up MPIO Driver configuration");
    }

    return err;
}

#if H5_VERSION_GE(1, 10, 0)
inline herr_t h5p_set_all_coll_metadata_ops(hid_t plist_id, hbool_t is_collective) {
    herr_t err = H5Pset_all_coll_metadata_ops(plist_id, is_collective);
    if (err < 0) {
        HDF5ErrMapper::ToException<FileException>("Unable to request collective metadata reads");
    }

    return err;
}

inline herr_t h5p_get_all_coll_metadata_ops(hid_t plist_id, hbool_t* is_collective) {
    herr_t err = H5Pget_all_coll_metadata_ops(plist_id, is_collective);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error loading MPI metadata read.");
    }

    return err;
}

inline herr_t h5p_set_coll_metadata_write(hid_t plist_id, hbool_t is_collective) {
    herr_t err = H5Pset_coll_metadata_write(plist_id, is_collective);

    if (err < 0) {
        HDF5ErrMapper::ToException<FileException>("Unable to request collective metadata writes");
    }

    return err;
}

inline herr_t h5p_get_coll_metadata_write(hid_t plist_id, hbool_t* is_collective) {
    herr_t err = H5Pget_coll_metadata_write(plist_id, is_collective);

    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error loading MPI metadata write.");
    }

    return err;
}
#endif
#endif

inline herr_t h5p_get_libver_bounds(hid_t plist_id, H5F_libver_t* low, H5F_libver_t* high) {
    herr_t err = H5Pget_libver_bounds(plist_id, low, high);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Unable to access file version bounds");
    }

    return err;
}

inline herr_t h5p_set_libver_bounds(hid_t plist_id, H5F_libver_t low, H5F_libver_t high) {
    herr_t err = H5Pset_libver_bounds(plist_id, low, high);

    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting file version bounds");
    }

    return err;
}

inline herr_t h5p_get_meta_block_size(hid_t fapl_id, hsize_t* size) {
    herr_t err = H5Pget_meta_block_size(fapl_id, size);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Unable to access file metadata block size");
    }

    return err;
}

inline herr_t h5p_set_meta_block_size(hid_t fapl_id, hsize_t size) {
    herr_t err = H5Pset_meta_block_size(fapl_id, size);

    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting metadata block size");
    }

    return err;
}

inline herr_t h5p_set_est_link_info(hid_t plist_id,
                                    unsigned est_num_entries,
                                    unsigned est_name_len) {
    herr_t err = H5Pset_est_link_info(plist_id, est_num_entries, est_name_len);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting estimated link info");
    }

    return err;
}

inline herr_t h5p_get_est_link_info(hid_t plist_id,
                                    unsigned* est_num_entries,
                                    unsigned* est_name_len) {
    herr_t err = H5Pget_est_link_info(plist_id, est_num_entries, est_name_len);

    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Unable to access group link size property");
    }

    return err;
}

inline herr_t h5p_set_chunk(hid_t plist_id, int ndims, const hsize_t dim[]) {
    herr_t err = H5Pset_chunk(plist_id, ndims, dim);

    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting chunk property");
    }

    return err;
}

inline int h5p_get_chunk(hid_t plist_id, int max_ndims, hsize_t dim[]) {
    int chunk_dims = H5Pget_chunk(plist_id, max_ndims, dim);
    if (chunk_dims < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error getting chunk size");
    }
    return chunk_dims;
}

inline htri_t h5z_filter_avail(H5Z_filter_t id) {
    htri_t tri = H5Zfilter_avail(id);
    if (tri < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error checking filter availability");
    }
    return tri;
}

inline herr_t h5p_set_deflate(hid_t plist_id, unsigned level) {
    herr_t err = H5Pset_deflate(plist_id, level);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting deflate property");
    }
    return err;
}

inline herr_t h5p_set_szip(hid_t plist_id, unsigned options_mask, unsigned pixels_per_block) {
    herr_t err = H5Pset_szip(plist_id, options_mask, pixels_per_block);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting szip property");
    }
    return err;
}

inline herr_t h5p_set_shuffle(hid_t plist_id) {
    herr_t err = H5Pset_shuffle(plist_id);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting shuffle property");
    }
    return err;
}

inline herr_t h5p_get_alloc_time(hid_t plist_id, H5D_alloc_time_t* alloc_time) {
    herr_t err = H5Pget_alloc_time(plist_id, alloc_time);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error getting allocation time");
    }
    return err;
}

inline herr_t h5p_set_alloc_time(hid_t plist_id, H5D_alloc_time_t alloc_time) {
    herr_t err = H5Pset_alloc_time(plist_id, alloc_time);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting allocation time");
    }
    return err;
}

inline herr_t h5p_get_chunk_cache(hid_t dapl_id,
                                  size_t* rdcc_nslots,
                                  size_t* rdcc_nbytes,
                                  double* rdcc_w0) {
    herr_t err = H5Pget_chunk_cache(dapl_id, rdcc_nslots, rdcc_nbytes, rdcc_w0);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error getting dataset cache parameters");
    }
    return err;
}

inline herr_t h5p_set_chunk_cache(hid_t dapl_id,
                                  size_t rdcc_nslots,
                                  size_t rdcc_nbytes,
                                  double rdcc_w0) {
    herr_t err = H5Pset_chunk_cache(dapl_id, rdcc_nslots, rdcc_nbytes, rdcc_w0);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting dataset cache parameters");
    }
    return err;
}

inline herr_t h5p_set_create_intermediate_group(hid_t plist_id, unsigned crt_intmd) {
    herr_t err = H5Pset_create_intermediate_group(plist_id, crt_intmd);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>(
            "Error setting property for create intermediate groups");
    }
    return err;
}

inline herr_t h5p_get_create_intermediate_group(hid_t plist_id, unsigned* crt_intmd) {
    herr_t err = H5Pget_create_intermediate_group(plist_id, crt_intmd);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>(
            "Error getting property for create intermediate groups");
    }
    return err;
}

#ifdef H5_HAVE_PARALLEL
inline herr_t h5p_set_dxpl_mpio(hid_t dxpl_id, H5FD_mpio_xfer_t xfer_mode) {
    herr_t err = H5Pset_dxpl_mpio(dxpl_id, xfer_mode);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting H5Pset_dxpl_mpio.");
    }
    return err;
}

inline herr_t h5p_get_dxpl_mpio(hid_t dxpl_id, H5FD_mpio_xfer_t* xfer_mode) {
    herr_t err = H5Pget_dxpl_mpio(dxpl_id, xfer_mode);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error getting H5Pset_dxpl_mpio.");
    }
    return err;
}

inline herr_t h5p_get_mpio_no_collective_cause(hid_t plist_id,
                                               uint32_t* local_no_collective_cause,
                                               uint32_t* global_no_collective_cause) {
    herr_t err = H5Pget_mpio_no_collective_cause(plist_id,
                                                 local_no_collective_cause,
                                                 global_no_collective_cause);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Failed to check mpio_no_collective_cause.");
    }
    return err;
}

#endif

inline herr_t h5p_set_link_creation_order(hid_t plist_id, unsigned crt_order_flags) {
    herr_t err = H5Pset_link_creation_order(plist_id, crt_order_flags);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>("Error setting LinkCreationOrder.");
    }
    return err;
}

inline herr_t h5p_get_link_creation_order(hid_t plist_id, unsigned* crt_order_flags) {
    herr_t err = H5Pget_link_creation_order(plist_id, crt_order_flags);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>(
            "Error getting property for link creation order");
    }
    return err;
}

inline herr_t h5p_get_attr_phase_change(hid_t plist_id,
                                        unsigned* max_compact,
                                        unsigned* min_dense) {
    herr_t err = H5Pget_attr_phase_change(plist_id, max_compact, min_dense);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>(
            "Error getting property for attribute phase change");
    }
    return err;
}

inline herr_t h5p_set_attr_phase_change(hid_t plist_id, unsigned max_compact, unsigned min_dense) {
    herr_t err = H5Pset_attr_phase_change(plist_id, max_compact, min_dense);
    if (err < 0) {
        HDF5ErrMapper::ToException<PropertyException>(
            "Error getting property for attribute phase change");
    }
    return err;
}


}  // namespace detail
}  // namespace HighFive
