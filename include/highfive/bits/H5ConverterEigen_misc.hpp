/*
 *  Copyright (c), 2020, EPFL - Blue Brain Project
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <Eigen/Eigen>

namespace HighFive {

namespace details {


#ifdef H5_USE_BOOST
// compute size for  boost::multi_array of Eigens
template <typename T, size_t Dims>
inline size_t compute_total_size(const boost::multi_array<T, Dims>& vec) {
    return std::accumulate(
        vec.origin(), vec.origin() + vec.num_elements(), size_t{0u}, [](size_t so_far, const T& v) {
            return so_far + static_cast<size_t>(v.rows()) * static_cast<size_t>(v.cols());
        });
}
#endif


#ifdef H5_USE_BOOST
template <typename T, int M, int N, std::size_t Dims>
struct data_converter<boost::multi_array<Eigen::Matrix<T, M, N>, Dims>, void> {
    using MultiArrayEigen = typename boost::multi_array<Eigen::Matrix<T, M, N>, Dims>;

    inline data_converter(const DataSpace& space)
        : _dims(space.getDimensions())
        , _space(space) {
        assert(_dims.size() == Dims + 2);
    }

    inline T* transform_read(const MultiArrayEigen& /*array*/) {
        _vec_align.resize(compute_total_size(_space.getDimensions()));
        return _vec_align.data();
    }

    inline const T* transform_write(const MultiArrayEigen& array) {
        _vec_align.reserve(compute_total_size(array));
        for (auto e = array.origin(); e < array.origin() + array.num_elements(); ++e) {
            std::copy(e->data(), e->data() + e->size(), std::back_inserter(_vec_align));
        }
        return _vec_align.data();
    }

    inline void process_result(MultiArrayEigen& vec) {
        T* start = _vec_align.data();
        if (M != -1 && N != -1) {
            for (auto v = vec.origin(); v < vec.origin() + vec.num_elements(); ++v) {
                *v = Eigen::Map<Eigen::Matrix<T, M, N>>(start, v->rows(), v->cols());
                start += v->rows() * v->cols();
            }
        } else {
            if (vec.origin()->rows() > 0 && vec.origin()->cols() > 0) {
                const auto VEC_M = vec.origin()->rows(), VEC_N = vec.origin()->cols();
                for (auto v = vec.origin(); v < vec.origin() + vec.num_elements(); ++v) {
                    assert(v->rows() == VEC_M && v->cols() == VEC_N);
                    *v = Eigen::Map<Eigen::Matrix<T, M, N>>(start, VEC_M, VEC_N);
                    start += VEC_M * VEC_N;
                }
            } else {
                throw DataSetException(
                    "Dynamic size(-1) used without pre-defined multi_array data layout.\n"
                    "Initialize vector elements using  MatrixXd::Zero");
            }
        }
    }

    std::vector<size_t> _dims;
    const DataSpace& _space;
    std::vector<typename inspector<T>::base_type> _vec_align;
};
#endif  // H5_USE_BOOST

}  // namespace details

}  // namespace HighFive
