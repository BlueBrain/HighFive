#pragma once

#include <H5Rpublic.h>

namespace HighFive {
namespace detail {
inline herr_t h5r_create(void* ref,
                         hid_t loc_id,
                         const char* name,
                         H5R_type_t ref_type,
                         hid_t space_id) {
    herr_t err = H5Rcreate(ref, loc_id, name, ref_type, space_id);
    if (err < 0) {
        HDF5ErrMapper::ToException<ReferenceException>(
            std::string("Unable to create the reference for \"") + name + "\":");
    }

    return err;
}

#if (H5Rdereference_vers == 2)
inline hid_t h5r_dereference(hid_t obj_id, hid_t oapl_id, H5R_type_t ref_type, const void* ref) {
    hid_t hid = H5Rdereference(obj_id, oapl_id, ref_type, ref);
    if (hid < 0) {
        HDF5ErrMapper::ToException<ReferenceException>("Unable to dereference.");
    }

    return hid;
}
#else
inline hid_t h5r_dereference(hid_t dataset, H5R_type_t ref_type, const void* ref) {
    hid_t hid = H5Rdereference(dataset, ref_type, ref);
    if (hid < 0) {
        HDF5ErrMapper::ToException<ReferenceException>("Unable to dereference.");
    }

    return hid;
}
#endif

}  // namespace detail
}  // namespace HighFive
