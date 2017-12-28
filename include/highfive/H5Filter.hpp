//
// Created by nwknoblauch on 12/2/17.
//

#ifndef HIGHFIVE_H5FILTER_HPP
#define HIGHFIVE_H5FILTER_HPP

#include "H5Object.hpp"
#include "H5PropertyList.hpp"

#ifdef H5_USE_EIGEN

#include <eigen3/Eigen/Core>

#endif

namespace HighFive {

///
/// \brief Generic HDF5 property List
///
    class Filter {
    public:
        Filter(const std::vector<size_t> &chunk_dims, const hid_t filter_id, const int r,
               const bool doTranspose = false);

#ifdef H5_USE_EIGEN

        template<typename Scalar, int RowsAtCompileTime, int ColsAtCompileTime, int Options>
        Filter(const std::vector<size_t> &chunk_dims,
               const Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime, Options> &mat, const hid_t filter_id,
               const bool doTranspose = false);

#endif
        hid_t getId() const;

    protected:
        // protected constructor
        hid_t _hid;


    };

} // HighFive

#include "bits/H5Filter_misc.hpp"

#endif //HIGHFIVE_H5FILTER_HPP
