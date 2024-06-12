#pragma once

#include <H5Ipublic.h>
#include <H5Tpublic.h>

namespace HighFive {
namespace detail {

inline hid_t h5o_open(hid_t loc_id, const char* name, hid_t lapl_id) {
    hid_t hid = H5Oopen(loc_id, name, lapl_id);
    if (hid < 0) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to open \"") + name + "\":");
    }

    return hid;
}

inline herr_t h5o_close(hid_t id) {
    herr_t err = H5Oclose(id);
    if (err < 0) {
        HDF5ErrMapper::ToException<ObjectException>("Unable to close object.");
    }

    return err;
}

}  // namespace detail
}  // namespace HighFive
