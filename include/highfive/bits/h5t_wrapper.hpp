#pragma once

#include <H5Ipublic.h>
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

inline int h5t_get_nmembers(hid_t hid) {
    auto result = H5Tget_nmembers(hid);

    if (result < 0) {
        throw DataTypeException("Could not get members of compound datatype");
    }

    return result;
}

inline char* h5t_get_member_name(hid_t type_id, unsigned membno) {
    char* name = H5Tget_member_name(type_id, membno);
    if (name == nullptr) {
        throw DataTypeException("Failed to get member names of compound datatype");
    }

    return name;
}


inline size_t h5t_get_member_offset(hid_t type_id, unsigned membno) {
    // Note, this function is peculiar. On failure it returns 0, yet 0 is also
    // what's returned on failure.
    return H5Tget_member_offset(type_id, membno);
}

inline hid_t h5t_get_member_type(hid_t type_id, unsigned membno) {
    hid_t member_id = H5Tget_member_type(type_id, membno);

    if (member_id < 0) {
        throw DataTypeException("Failed to get member type of compound datatype");
    }

    return member_id;
}

#if H5_VERSION_GE(1, 12, 0)
inline herr_t h5t_reclaim(hid_t type_id, hid_t space_id, hid_t plist_id, void* buf) {
    herr_t err = H5Treclaim(type_id, space_id, plist_id, buf);
    if (err < 0) {
        throw DataTypeException("Failed to reclaim HDF5 internal memory");
    }

    return err;
}
#endif

inline H5T_class_t h5t_get_class(hid_t type_id) {
    H5T_class_t class_id = H5Tget_class(type_id);
    if (class_id == H5T_NO_CLASS) {
        throw DataTypeException("Failed to get class of type");
    }

    return class_id;
}

inline htri_t h5t_equal(hid_t type1_id, hid_t type2_id) {
    htri_t equal = H5Tequal(type1_id, type2_id);
    if (equal < 0) {
        throw DataTypeException("Failed to compare two datatypes");
    }

    return equal;
}

inline htri_t h5t_is_variable_str(hid_t type_id) {
    htri_t is_variable = H5Tis_variable_str(type_id);
    if (is_variable < 0) {
        HDF5ErrMapper::ToException<DataTypeException>(
            "Failed to check if string is variable length");
    }
    return is_variable;
}

inline herr_t h5t_set_fields(hid_t type_id,
                             size_t spos,
                             size_t epos,
                             size_t esize,
                             size_t mpos,
                             size_t msize) {
    herr_t err = H5Tset_fields(type_id, spos, epos, esize, mpos, msize);
    if (err < 0) {
        HDF5ErrMapper::ToException<DataTypeException>(
            "Failed to create custom floating point data type");
    }
    return err;
}

inline herr_t h5t_set_ebias(hid_t type_id, size_t ebias) {
    herr_t err = H5Tset_ebias(type_id, ebias);
    if (err < 0) {
        HDF5ErrMapper::ToException<DataTypeException>(
            "Failed to exponent bias of floating point data type");
    }

    return err;
}

inline hid_t h5t_create(H5T_class_t type, size_t size) {
    hid_t type_id = H5Tcreate(type, size);
    if (type_id == H5I_INVALID_HID) {
        HDF5ErrMapper::ToException<DataTypeException>("Failed to datatype");
    }

    return type_id;
}

inline herr_t h5t_insert(hid_t parent_id, const char* name, size_t offset, hid_t member_id) {
    herr_t err = H5Tinsert(parent_id, name, offset, member_id);
    if (err < 0) {
        HDF5ErrMapper::ToException<DataTypeException>("Failed to not add new member to datatype");
    }

    return err;
}

inline herr_t h5t_commit2(hid_t loc_id,
                          const char* name,
                          hid_t type_id,
                          hid_t lcpl_id,
                          hid_t tcpl_id,
                          hid_t tapl_id) {
    herr_t err = H5Tcommit2(loc_id, name, type_id, lcpl_id, tcpl_id, tapl_id);
    if (err < 0) {
        HDF5ErrMapper::ToException<DataTypeException>("Failed to commit datatype");
    }

    return err;
}

inline herr_t h5t_close(hid_t type_id) {
    auto err = H5Tclose(type_id);
    if (err < 0) {
        HDF5ErrMapper::ToException<DataTypeException>("Failed to close datatype");
    }

    return err;
}

inline hid_t h5t_enum_create(hid_t base_id) {
    hid_t type_id = H5Tenum_create(base_id);
    if (type_id == H5I_INVALID_HID) {
        HDF5ErrMapper::ToException<DataTypeException>("Failed to create new enum datatype");
    }
    return type_id;
}

inline herr_t h5t_enum_insert(hid_t type, const char* name, const void* value) {
    herr_t err = H5Tenum_insert(type, name, value);
    if (err < 0) {
        HDF5ErrMapper::ToException<DataTypeException>(
            "Failed to add new member to this enum datatype");
    }
    return err;
}

inline hid_t h5t_open2(hid_t loc_id, const char* name, hid_t tapl_id) {
    hid_t datatype_id = H5Topen2(loc_id, name, tapl_id);
    if (datatype_id == H5I_INVALID_HID) {
        HDF5ErrMapper::ToException<DataTypeException>(
            std::string("Unable to open the datatype \"") + name + "\":");
    }

    return datatype_id;
}

}  // namespace detail
}  // namespace HighFive
