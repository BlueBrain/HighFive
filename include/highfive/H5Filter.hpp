//
// Created by nwknoblauch on 12/2/17.
//

#ifndef HIGHFIVE_H5FILTER_HPP
#define HIGHFIVE_H5FILTER_HPP

#include "H5Object.hpp"
#include "H5PropertyList.hpp"

namespace HighFive {

///
/// \brief Generic HDF5 property List
///
    class Filter {
    public:
        Filter(const std::vector<size_t> &chunk_dims, const hid_t filter_id, const int r);

        hid_t getId() const;

    protected:
        // protected constructor
        hid_t _hid;


    };

} // HighFive

#include "bits/H5Filter_misc.hpp"

#endif //HIGHFIVE_H5FILTER_HPP
