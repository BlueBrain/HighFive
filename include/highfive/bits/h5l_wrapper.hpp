#pragma once

#include <H5Lpublic.h>

namespace HighFive {
namespace detail {

inline herr_t h5l_create_external(const char* file_name,
                                  const char* obj_name,
                                  hid_t link_loc_id,
                                  const char* link_name,
                                  hid_t lcpl_id,
                                  hid_t lapl_id) {
    herr_t err = H5Lcreate_external(file_name, obj_name, link_loc_id, link_name, lcpl_id, lapl_id);
    if (err < 0) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to create external link: "));
    }

    return err;
}

inline herr_t h5l_create_soft(const char* link_target,
                              hid_t link_loc_id,
                              const char* link_name,
                              hid_t lcpl_id,
                              hid_t lapl_id) {
    herr_t err = H5Lcreate_soft(link_target, link_loc_id, link_name, lcpl_id, lapl_id);
    if (err < 0) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to create soft link: "));
    }

    return err;
}

inline herr_t h5l_create_hard(hid_t cur_loc,
                              const char* cur_name,
                              hid_t dst_loc,
                              const char* dst_name,
                              hid_t lcpl_id,
                              hid_t lapl_id) {
    herr_t err = H5Lcreate_hard(cur_loc, cur_name, dst_loc, dst_name, lcpl_id, lapl_id);
    if (err < 0) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to create hard link: "));
    }

    return err;
}

inline herr_t h5l_get_info(hid_t loc_id, const char* name, H5L_info_t* linfo, hid_t lapl_id) {
    herr_t err = H5Lget_info(loc_id, name, linfo, lapl_id);
    if (err < 0) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to obtain info for link "));
    }

    return err;
}

inline herr_t h5l_delete(hid_t loc_id, const char* name, hid_t lapl_id) {
    herr_t err = H5Ldelete(loc_id, name, lapl_id);
    if (err < 0) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Invalid name for unlink() "));
    }

    return err;
}

inline htri_t h5l_exists(hid_t loc_id, const char* name, hid_t lapl_id) {
    htri_t tri = H5Lexists(loc_id, name, lapl_id);
    if (tri < 0) {
        HDF5ErrMapper::ToException<GroupException>("Invalid link for exist()");
    }

    return tri;
}

namespace nothrow {

inline htri_t h5l_exists(hid_t loc_id, const char* name, hid_t lapl_id) {
    return H5Lexists(loc_id, name, lapl_id);
}

}  // namespace nothrow

inline herr_t h5l_iterate(hid_t grp_id,
                          H5_index_t idx_type,
                          H5_iter_order_t order,
                          hsize_t* idx,
                          H5L_iterate_t op,
                          void* op_data) {
    herr_t err = H5Literate(grp_id, idx_type, order, idx, op, op_data);
    if (err < 0) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to list objects in group"));
    }
    return err;
}

inline herr_t h5l_move(hid_t src_loc,
                       const char* src_name,
                       hid_t dst_loc,
                       const char* dst_name,
                       hid_t lcpl_id,
                       hid_t lapl_id) {
    herr_t err = H5Lmove(src_loc, src_name, dst_loc, dst_name, lcpl_id, lapl_id);

    if (err < 0) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to move link to \"") +
                                                   dst_name + "\":");
    }
    return err;
}

inline ssize_t h5l_get_name_by_idx(hid_t loc_id,
                                   const char* group_name,
                                   H5_index_t idx_type,
                                   H5_iter_order_t order,
                                   hsize_t n,
                                   char* name,
                                   size_t size,
                                   hid_t lapl_id) {
    ssize_t n_chars =
        H5Lget_name_by_idx(loc_id, group_name, idx_type, order, n, name, size, lapl_id);

    if (n_chars < 0) {
        HDF5ErrMapper::ToException<GroupException>(
            std::string("Unable to obtain link name from index."));
    }

    return n_chars;
}

}  // namespace detail
}  // namespace HighFive
