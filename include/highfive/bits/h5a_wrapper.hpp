#pragma once

#include <H5Apublic.h>
#include <H5Ipublic.h>

namespace HighFive {
namespace detail {

inline hid_t h5a_create2(hid_t loc_id,
                         char const* const attr_name,
                         hid_t type_id,
                         hid_t space_id,
                         hid_t acpl_id,
                         hid_t aapl_id) {
    auto attr_id = H5Acreate2(loc_id, attr_name, type_id, space_id, acpl_id, aapl_id);
    if (attr_id < 0) {
        HDF5ErrMapper::ToException<AttributeException>(
            std::string("Unable to create the attribute \"") + attr_name + "\":");
    }

    return attr_id;
}

inline void h5a_delete(hid_t loc_id, char const* const attr_name) {
    if (H5Adelete(loc_id, attr_name) < 0) {
        HDF5ErrMapper::ToException<AttributeException>(
            std::string("Unable to delete attribute \"") + attr_name + "\":");
    }
}

inline hid_t h5a_open(hid_t loc_id, char const* const attr_name, hid_t aapl_id) {
    const auto attr_id = H5Aopen(loc_id, attr_name, aapl_id);
    if (attr_id < 0) {
        HDF5ErrMapper::ToException<AttributeException>(
            std::string("Unable to open the attribute \"") + attr_name + "\":");
    }

    return attr_id;
}


inline int h5a_get_num_attrs(hid_t loc_id) {
    int res = H5Aget_num_attrs(loc_id);
    if (res < 0) {
        HDF5ErrMapper::ToException<AttributeException>(
            std::string("Unable to count attributes in existing group or file"));
    }

    return res;
}


inline void h5a_iterate2(hid_t loc_id,
                         H5_index_t idx_type,
                         H5_iter_order_t order,
                         hsize_t* idx,
                         H5A_operator2_t op,
                         void* op_data) {
    if (H5Aiterate2(loc_id, idx_type, order, idx, op, op_data) < 0) {
        HDF5ErrMapper::ToException<AttributeException>(std::string("Failed H5Aiterate2."));
    }
}

inline int h5a_exists(hid_t obj_id, char const* const attr_name) {
    int res = H5Aexists(obj_id, attr_name);
    if (res < 0) {
        HDF5ErrMapper::ToException<AttributeException>(
            std::string("Unable to check for attribute in group"));
    }

    return res;
}

inline ssize_t h5a_get_name(hid_t attr_id, size_t buf_size, char* buf) {
    ssize_t name_length = H5Aget_name(attr_id, buf_size, buf);
    if (name_length < 0) {
        HDF5ErrMapper::ToException<AttributeException>(
            std::string("Unable to get name of attribute"));
    }

    return name_length;
}


inline hid_t h5a_get_space(hid_t attr_id) {
    hid_t attr = H5Aget_space(attr_id);
    if (attr < 0) {
        HDF5ErrMapper::ToException<AttributeException>(
            std::string("Unable to get dataspace of attribute"));
    }

    return attr;
}

inline hsize_t h5a_get_storage_size(hid_t attr_id) {
    // Docs:
    //    Returns the amount of storage size allocated for the attribute;
    //    otherwise returns 0 (zero).
    return H5Aget_storage_size(attr_id);
}

inline hid_t h5a_get_type(hid_t attr_id) {
    hid_t type_id = H5Aget_type(attr_id);
    if (type_id == H5I_INVALID_HID) {
        HDF5ErrMapper::ToException<AttributeException>(
            std::string("Unable to get datatype of attribute"));
    }

    return type_id;
}

inline herr_t h5a_read(hid_t attr_id, hid_t type_id, void* buf) {
    herr_t err = H5Aread(attr_id, type_id, buf);
    if (err < 0) {
        HDF5ErrMapper::ToException<AttributeException>(std::string("Unable to read attribute"));
    }

    return err;
}

inline herr_t h5a_write(hid_t attr_id, hid_t type_id, void const* buf) {
    herr_t err = H5Awrite(attr_id, type_id, buf);
    if (err < 0) {
        HDF5ErrMapper::ToException<AttributeException>(std::string("Unable to write attribute"));
    }

    return err;
}

}  // namespace detail
}  // namespace HighFive
