#pragma once

#include <H5Ipublic.h>
#include <H5Spublic.h>
namespace HighFive {
namespace detail {

inline hid_t h5s_create_simple(int rank, const hsize_t dims[], const hsize_t maxdims[]) {
    hid_t space_id = H5Screate_simple(rank, dims, maxdims);
    if (space_id == H5I_INVALID_HID) {
        throw DataSpaceException("Unable to create simple dataspace");
    }

    return space_id;
}

inline hid_t h5s_create(H5S_class_t type) {
    hid_t space_id = H5Screate(type);

    if (space_id == H5I_INVALID_HID) {
        throw DataSpaceException("Unable to create dataspace");
    }

    return space_id;
}

inline hid_t h5s_copy(hid_t space_id) {
    hid_t copy_id = H5Scopy(space_id);

    if (copy_id < 0) {
        throw DataSpaceException("Unable to copy dataspace");
    }

    return copy_id;
}

inline herr_t h5s_select_none(hid_t spaceid) {
    herr_t err = H5Sselect_none(spaceid);
    if (err < 0) {
        HDF5ErrMapper::ToException<DataSpaceException>("Unable to select None space");
    }
    return err;
}

inline herr_t h5s_select_hyperslab(hid_t space_id,
                                   H5S_seloper_t op,
                                   const hsize_t start[],
                                   const hsize_t stride[],
                                   const hsize_t count[],
                                   const hsize_t block[]) {
    herr_t err = H5Sselect_hyperslab(space_id, op, start, stride, count, block);
    if (err < 0) {
        HDF5ErrMapper::ToException<DataSpaceException>("Unable to select hyperslab");
    }
    return err;
}

inline hssize_t h5s_get_select_npoints(hid_t spaceid) {
    hssize_t n_points = H5Sget_select_npoints(spaceid);
    if (n_points < 0) {
        HDF5ErrMapper::ToException<DataSpaceException>(
            "Unable to get number of points in selection");
    }
    return n_points;
}

inline herr_t h5s_select_elements(hid_t space_id,
                                  H5S_seloper_t op,
                                  size_t num_elem,
                                  const hsize_t* coord) {
    herr_t err = H5Sselect_elements(space_id, op, num_elem, coord);
    if (err < 0) {
        HDF5ErrMapper::ToException<DataSpaceException>("Unable to select elements");
    }
    return err;
}

inline int h5s_get_simple_extent_ndims(hid_t space_id) {
    int ndim = H5Sget_simple_extent_ndims(space_id);
    if (ndim < 0) {
        HDF5ErrMapper::ToException<DataSetException>(
            "Unable to get number of dimensions of dataspace");
    }
    return ndim;
}

inline herr_t h5s_get_simple_extent_dims(hid_t space_id, hsize_t dims[], hsize_t maxdims[]) {
    herr_t err = H5Sget_simple_extent_dims(space_id, dims, maxdims);
    if (err < 0) {
        HDF5ErrMapper::ToException<DataSetException>("Unable to get dimensions of dataspace");
    }
    return err;
}

inline hssize_t h5s_get_simple_extent_npoints(hid_t space_id) {
    hssize_t nelements = H5Sget_simple_extent_npoints(space_id);
    if (nelements < 0) {
        HDF5ErrMapper::ToException<DataSetException>(
            "Unable to get number of elements in dataspace");
    }

    return nelements;
}

inline H5S_class_t h5s_get_simple_extent_type(hid_t space_id) {
    H5S_class_t cls = H5Sget_simple_extent_type(space_id);
    if (cls == H5S_NO_CLASS) {
        HDF5ErrMapper::ToException<DataSpaceException>("Unable to get class of simple dataspace.");
    }

    return cls;
}

inline H5S_sel_type h5s_get_select_type(hid_t space_id) {
    H5S_sel_type type = H5Sget_select_type(space_id);
    if (type < 0) {
        HDF5ErrMapper::ToException<DataSpaceException>("Unable to get type of selection.");
    }

    return type;
}

#if H5_VERSION_GE(1, 10, 6)
inline hid_t h5s_combine_select(hid_t space1_id, H5S_seloper_t op, hid_t space2_id) {
    auto space_id = H5Scombine_select(space1_id, op, space2_id);
    if (space_id == H5I_INVALID_HID) {
        HDF5ErrMapper::ToException<DataSpaceException>("Unable to combine two selections.");
    }

    return space_id;
}
#endif


}  // namespace detail
}  // namespace HighFive
