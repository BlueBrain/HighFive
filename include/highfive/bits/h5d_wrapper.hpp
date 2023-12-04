#pragma once

#include <H5Dpublic.h>
#include <H5Ipublic.h>

namespace HighFive {
namespace detail {


#if !H5_VERSION_GE(1, 12, 0)
inline herr_t h5d_vlen_reclaim(hid_t type_id, hid_t space_id, hid_t dxpl_id, void* buf) {
    herr_t err = H5Dvlen_reclaim(type_id, space_id, dxpl_id, buf);
    if (err < 0) {
        throw DataSetException("Failed to reclaim HDF5 internal memory");
    }

    return err;
}
#endif

inline hsize_t h5d_get_storage_size(hid_t dset_id) {
    // Docs:
    //    H5Dget_storage_size() does not differentiate between 0 (zero), the
    //    value returned for the storage size of a dataset with no stored values,
    //    and 0 (zero), the value returned to indicate an error.
    return H5Dget_storage_size(dset_id);
}

inline hid_t h5d_get_space(hid_t dset_id) {
    hid_t dset = H5Dget_space(dset_id);
    if (dset == H5I_INVALID_HID) {
        HDF5ErrMapper::ToException<DataSetException>(
            std::string("Unable to get dataspace of the dataset"));
    }

    return dset;
}

inline hid_t h5d_get_type(hid_t dset_id) {
    hid_t type_id = H5Dget_type(dset_id);
    if (type_id == H5I_INVALID_HID) {
        HDF5ErrMapper::ToException<DataSetException>(
            std::string("Unable to get datatype of the dataset"));
    }

    return type_id;
}

inline herr_t h5d_read(hid_t dset_id,
                       hid_t mem_type_id,
                       hid_t mem_space_id,
                       hid_t file_space_id,
                       hid_t dxpl_id,
                       void* buf) {
    herr_t err = H5Dread(dset_id, mem_type_id, mem_space_id, file_space_id, dxpl_id, buf);
    if (err < 0) {
        HDF5ErrMapper::ToException<DataSetException>(std::string("Unable to read the dataset"));
    }

    return err;
}

inline herr_t h5d_write(hid_t dset_id,
                        hid_t mem_type_id,
                        hid_t mem_space_id,
                        hid_t file_space_id,
                        hid_t dxpl_id,
                        const void* buf) {
    herr_t err = H5Dwrite(dset_id, mem_type_id, mem_space_id, file_space_id, dxpl_id, buf);
    if (err < 0) {
        HDF5ErrMapper::ToException<DataSetException>(std::string("Unable to write the dataset"));
    }

    return err;
}

inline haddr_t h5d_get_offset(hid_t dset_id) {
    uint64_t addr = H5Dget_offset(dset_id);
    if (addr == HADDR_UNDEF) {
        HDF5ErrMapper::ToException<DataSetException>("Cannot get offset of DataSet.");
    }
    return addr;
}


inline herr_t h5d_set_extent(hid_t dset_id, const hsize_t size[]) {
    herr_t err = H5Dset_extent(dset_id, size);
    if (H5Dset_extent(dset_id, size) < 0) {
        HDF5ErrMapper::ToException<DataSetException>("Could not resize dataset.");
    }

    return err;
}

inline hid_t h5d_create2(hid_t loc_id,
                         const char* name,
                         hid_t type_id,
                         hid_t space_id,
                         hid_t lcpl_id,
                         hid_t dcpl_id,
                         hid_t dapl_id) {
    hid_t dataset_id = H5Dcreate2(loc_id, name, type_id, space_id, lcpl_id, dcpl_id, dapl_id);

    if (dataset_id == H5I_INVALID_HID) {
        HDF5ErrMapper::ToException<DataSetException>(
            std::string("Failed to create the dataset \"") + name + "\":");
    }

    return dataset_id;
}

inline hid_t h5d_open2(hid_t loc_id, const char* name, hid_t dapl_id) {
    hid_t dataset_id = H5Dopen2(loc_id, name, dapl_id);

    if (dataset_id == H5I_INVALID_HID) {
        HDF5ErrMapper::ToException<DataSetException>(std::string("Unable to open the dataset \"") +
                                                     name + "\":");
    }

    return dataset_id;
}


}  // namespace detail
}  // namespace HighFive
