#pragma once

#include <H5Fpublic.h>
namespace HighFive {
namespace detail {
namespace nothrow {
inline hid_t h5f_open(const char* filename, unsigned flags, hid_t fapl_id) {
    return H5Fopen(filename, flags, fapl_id);
}
}  // namespace nothrow

inline hid_t h5f_create(const char* filename, unsigned flags, hid_t fcpl_id, hid_t fapl_id) {
    hid_t file_id = H5Fcreate(filename, flags, fcpl_id, fapl_id);

    if (file_id == H5I_INVALID_HID) {
        HDF5ErrMapper::ToException<FileException>(std::string("Failed to create file ") + filename);
    }
    return file_id;
}

inline ssize_t h5f_get_name(hid_t obj_id, char* name, size_t size) {
    ssize_t nread = H5Fget_name(obj_id, name, size);
    if (nread < 0) {
        HDF5ErrMapper::ToException<FileException>(std::string("Failed to get file from id"));
    }

    return nread;
}

inline herr_t h5f_flush(hid_t object_id, H5F_scope_t scope) {
    herr_t err = H5Fflush(object_id, scope);
    if (err < 0) {
        HDF5ErrMapper::ToException<FileException>(std::string("Failed to flush file"));
    }

    return err;
}

inline herr_t h5f_get_filesize(hid_t file_id, hsize_t* size) {
    herr_t err = H5Fget_filesize(file_id, size);
    if (err < 0) {
        HDF5ErrMapper::ToException<FileException>(std::string("Unable to retrieve size of file"));
    }

    return err;
}

inline hssize_t h5f_get_freespace(hid_t file_id) {
    hssize_t free_space = H5Fget_freespace(file_id);
    if (free_space < 0) {
        HDF5ErrMapper::ToException<FileException>(
            std::string("Unable to retrieve unused space of file "));
    }
    return free_space;
}

}  // namespace detail
}  // namespace HighFive
