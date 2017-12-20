//
// Created by nwknoblauch on 12/2/17.
//

#ifndef HIGHFIVE_H5FILTER_MISC_HPP
#define HIGHFIVE_H5FILTER_MISC_HPP

#include "../H5Filter.hpp"

namespace HighFive {

    inline Filter::Filter(const std::vector<size_t> &chunk_dims, const hid_t filter_id, const int r) {
        if (r < 0) {
            HDF5ErrMapper::ToException<FilterException>(
                    "Filter Improperly registered");
        }
        _hid = H5Pcreate(H5P_DATASET_CREATE);
        if (_hid < 0) {
            HDF5ErrMapper::ToException<FilterException>(
                    "Unable to get create PropertyList");
        }

        const size_t c_size = chunk_dims.size();
        std::vector<hsize_t> nchunk_dims(c_size);
        std::copy(chunk_dims.begin(), chunk_dims.end(), nchunk_dims.begin());
        auto rr = H5Pset_chunk(_hid, c_size, nchunk_dims.data());
        if (rr < 0) {
            HDF5ErrMapper::ToException<FilterException>(
                    "Unable to set chunk size");
        }
        rr = H5Pset_filter(_hid, filter_id, H5Z_FLAG_OPTIONAL, 0, NULL);
        if (rr < 0) {
            HDF5ErrMapper::ToException<FilterException>(
                    "Unable to set filter");
        }
    }

    inline hid_t Filter::getId() const {
        return _hid;
    }

} // HighFive



#endif //HIGHFIVE_H5FILTER_MISC_HPP

