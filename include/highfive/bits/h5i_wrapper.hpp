#pragma once

#include <H5Ipublic.h>

namespace HighFive {
namespace detail {
inline int h5i_inc_ref(hid_t id) {
    auto count = H5Iinc_ref(id);

    if (count < 0) {
        throw ObjectException("Failed to increase reference count of HID");
    }

    return count;
}

namespace nothrow {

inline int h5i_dec_ref(hid_t id) {
    return H5Idec_ref(id);
}

}  // namespace nothrow

inline int h5i_dec_ref(hid_t id) {
    int count = H5Idec_ref(id);
    if (count < 0) {
        throw ObjectException("Failed to decrease reference count of HID");
    }

    return count;
}

namespace nothrow {
inline htri_t h5i_is_valid(hid_t id) {
    return H5Iis_valid(id);
}

}  // namespace nothrow

inline htri_t h5i_is_valid(hid_t id) {
    htri_t tri = H5Iis_valid(id);
    if (tri < 0) {
        throw ObjectException("Failed to check if HID is valid");
    }

    return tri;
}

inline H5I_type_t h5i_get_type(hid_t id) {
    H5I_type_t type = H5Iget_type(id);
    if (type == H5I_BADID) {
        HDF5ErrMapper::ToException<ObjectException>("Failed to get type of HID");
    }

    return type;
}

template <class Exception>
inline hid_t h5i_get_file_id(hid_t id) {
    hid_t file_id = H5Iget_file_id(id);
    if (file_id < 0) {
        HDF5ErrMapper::ToException<Exception>("Failed not obtain file HID of object");
    }

    return file_id;
}

inline ssize_t h5i_get_name(hid_t id, char* name, size_t size) {
    ssize_t n_chars = H5Iget_name(id, name, size);
    if (n_chars < 0) {
        HDF5ErrMapper::ToException<ObjectException>("Failed to get name of HID.");
    }

    return n_chars;
}

}  // namespace detail
}  // namespace HighFive
