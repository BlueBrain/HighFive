#pragma once

#include <H5Tpublic.h>

namespace HighFive {
namespace detail {

inline hid_t h5t_copy(hid_t original) {
    auto copy = H5Tcopy(original);
    if (copy == H5I_INVALID_HID) {
        HDF5ErrMapper::ToException<DataTypeException>("Error copying datatype.");
    }

    return copy;
}

inline hsize_t h5t_get_size(hid_t hid) {
    hsize_t size = H5Tget_size(hid);
    if (size == 0) {
        HDF5ErrMapper::ToException<DataTypeException>("Error getting size of datatype.");
    }

    return size;
}

inline H5T_cset_t h5t_get_cset(hid_t hid) {
    auto cset = H5Tget_cset(hid);
    if (cset == H5T_CSET_ERROR) {
        HDF5ErrMapper::ToException<DataTypeException>("Error getting cset of datatype.");
    }

    return cset;
}

inline H5T_str_t h5t_get_strpad(hid_t hid) {
    auto strpad = H5Tget_strpad(hid);
    if (strpad == H5T_STR_ERROR) {
        HDF5ErrMapper::ToException<DataTypeException>("Error getting strpad of datatype.");
    }

    return strpad;
}

inline void h5t_set_size(hid_t hid, hsize_t size) {
    if (H5Tset_size(hid, size) < 0) {
        HDF5ErrMapper::ToException<DataTypeException>("Error setting size of datatype.");
    }
}

inline void h5t_set_cset(hid_t hid, H5T_cset_t cset) {
    if (H5Tset_cset(hid, cset) < 0) {
        HDF5ErrMapper::ToException<DataTypeException>("Error setting cset of datatype.");
    }
}

inline void h5t_set_strpad(hid_t hid, H5T_str_t strpad) {
    if (H5Tset_strpad(hid, strpad) < 0) {
        HDF5ErrMapper::ToException<DataTypeException>("Error setting strpad of datatype.");
    }
}

inline H5T_class_t h5t_get_class(hid_t type_id) {
    H5T_class_t class_id = H5Tget_class(type_id);
    if (class_id == H5T_NO_CLASS) {
        throw DataTypeException("Failed to get class of type");
    }

    return class_id;
}

}  // namespace detail
}  // namespace HighFive
