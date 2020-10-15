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


//compute size for single Eigen Matrix
template <typename T, int M, int N>
inline size_t compute_total_size(const Eigen::Matrix<T,M,N>& matrix) {
    return static_cast<size_t>(matrix.rows()) * static_cast<size_t>(matrix.cols());
}

//compute size for  std::vector of Eigens
template <typename T, int M, int N>
inline size_t compute_total_size(const std::vector<Eigen::Matrix<T,M,N>>& vec) {
    return vec.size() * compute_total_size(vec[0]);
}

#ifdef H5_USE_BOOST
// compute size for  boost::multi_array of Eigens
template <typename T, size_t Dims>
inline size_t compute_total_size(const boost::multi_array<T, Dims>& vec) {
    return std::accumulate(vec.origin(), vec.origin() + vec.num_elements(), size_t{0u},
        [](size_t so_far, const T& v) {
            return so_far + static_cast<size_t>(v.rows()) * static_cast<size_t>(v.cols());
        });
}
#endif

// apply conversion to eigen matrix
template <typename T, int M, int N>
struct data_converter<Eigen::Matrix<T, M, N>, void> {

    typedef Eigen::Matrix<T, M, N> MatrixTMN;

    inline data_converter(const DataSpace& space)
        : _dims(space.getDimensions()) {
        assert(_dims.size() == 2);
    }

    inline T* transform_read(MatrixTMN& array) {
        if (_dims[0] != static_cast<size_t>(array.rows()) ||
            _dims[1] != static_cast<size_t>(array.cols())) {
            array.resize(static_cast<typename MatrixTMN::Index>(_dims[0]),
                         static_cast<typename MatrixTMN::Index>(_dims[1]));
        }
        return array.data();
    }

    inline const T* transform_write(const MatrixTMN& array) {
        return array.data();
    }

    inline void process_result(MatrixTMN&) {}

    std::vector<size_t> _dims;
};


template <typename T, int M, int N>
inline void vectors_to_single_buffer(const std::vector<Eigen::Matrix<T,M,N>>& vec,
                                     const std::vector<size_t>& dims,
                                     const size_t current_dim,
                                     std::vector<T>& buffer) {

    check_dimensions_vector(vec.size(), dims[current_dim], current_dim);
    for (const auto& k : vec) {
        std::copy(k.data(), k.data() + k.size(), std::back_inserter(buffer));
    }
}

// apply conversion to std::vector of eigen matrix
template <typename T, int M, int N>
struct data_converter<std::vector<Eigen::Matrix<T,M,N>>, void> {

    typedef Eigen::Matrix<T, M, N> MatrixTMN;

    inline data_converter(const DataSpace& space)
        : _dims(space.getDimensions()), _space(space) {
        assert(_dims.size() == 3);
    }

    inline T * transform_read(std::vector<MatrixTMN>& /* vec */) {
        _vec_align.resize(compute_total_size(_space.getDimensions()));
        return _vec_align.data();
    }

    inline const T* transform_write(const std::vector<MatrixTMN>& vec) {
        _vec_align.reserve(compute_total_size(vec));
        vectors_to_single_buffer<T, M, N>(vec, _dims, 0, _vec_align);
        return _vec_align.data();
    }

    inline void process_result(std::vector<MatrixTMN>& vec) {
        T* start = _vec_align.data();
        if (vec.size() > 0) {
            for(auto& v : vec){
                v = Eigen::Map<MatrixTMN>(start, v.rows(), v.cols());
                start += v.rows()*v.cols();
            }
        }
        else if (M == -1 || N == -1) {
            std::ostringstream ss;
            ss << "Dynamic size(-1) used without pre-defined vector data layout.\n"
               << "Initiliaze vector elements using Zero, i.e.:\n"
               << "\t vector<MatrixXd> vec(5, MatrixXd::Zero(20,5))";
            throw DataSetException(ss.str());
        }
        else {
            for (size_t i = 0; i < _dims[0]; ++i) {
                vec.emplace_back(Eigen::Map<MatrixTMN>(start, M, N));
                start += M * N;
            }
        }
    }

    std::vector<size_t> _dims;
    std::vector<typename inspector<T>::base_type> _vec_align;
    const DataSpace& _space;
};

#ifdef H5_USE_BOOST
template <typename T, int M, int N, std::size_t Dims>
struct data_converter<boost::multi_array<Eigen::Matrix<T, M, N>, Dims>, void> {
    typedef typename boost::multi_array<Eigen::Matrix<T, M, N>, Dims> MultiArrayEigen;

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
