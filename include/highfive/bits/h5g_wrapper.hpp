#pragma once

#include <H5Dpublic.h>
#include <H5Ipublic.h>

#include <highfive/H5Exception.hpp>

namespace HighFive {
namespace detail {

inline hid_t h5g_create2(hid_t loc_id,
                         const char* name,
                         hid_t lcpl_id,
                         hid_t gcpl_id,
                         hid_t gapl_id) {
    hid_t group_id = H5Gcreate2(loc_id, name, lcpl_id, gcpl_id, gapl_id);
    if (group_id == H5I_INVALID_HID) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to create the group \"") +
                                                   name + "\":");
    }

    return group_id;
}

inline hid_t h5g_open2(hid_t loc_id, const char* name, hid_t gapl_id) {
    hid_t group_id = H5Gopen2(loc_id, name, gapl_id);
    if (group_id == H5I_INVALID_HID) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to open the group \"") +
                                                   name + "\":");
    }
    return group_id;
}

inline herr_t h5g_get_num_objs(hid_t loc_id, hsize_t* num_objs) {
    herr_t err = H5Gget_num_objs(loc_id, num_objs);
    if (err < 0) {
        HDF5ErrMapper::ToException<GroupException>(
            std::string("Unable to count objects in existing group or file"));
    }

    return err;
}


}  // namespace detail
}  // namespace HighFive
