/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5CONVERTER_MISC_HPP
#define H5CONVERTER_MISC_HPP

#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <sstream>
#include <string>

#ifdef H5_USE_BOOST
#include <boost/multi_array.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <type_traits>
#endif


#ifdef H5_USE_EIGEN

#include <eigen3/Eigen/Core>

#endif

#include <H5Dpublic.h>
#include <H5Ppublic.h>

#include "../H5DataSpace.hpp"
#include "../H5DataType.hpp"

#include "H5Utils.hpp"

#if H5_USE_CXX11
#include <type_traits>
#endif

namespace HighFive {

namespace details {

inline void check_dimensions_vector(size_t size_vec, size_t size_dataset,
                                    size_t dimension) {
    if (size_vec != size_dataset) {
        std::ostringstream ss;
        ss << "Mismatch between vector size (" << size_vec
           << ") and dataset size (" << size_dataset;
        ss << ") on dimension " << dimension;
        throw DataSetException(ss.str());
    }
}

// copy multi dimensional vector in C++ in one C-style multi dimensional buffer
template <typename T>
inline void vectors_to_single_buffer(const std::vector<T>& vec_single_dim,
                                     const std::vector<size_t>& dims,
                                     size_t current_dim,
                                     std::vector<T>& buffer) {

    check_dimensions_vector(vec_single_dim.size(), dims[current_dim],
                            current_dim);
    buffer.insert(buffer.end(), vec_single_dim.begin(), vec_single_dim.end());
}

template <typename T>
inline void
vectors_to_single_buffer(const std::vector<T>& vec_multi_dim,
                         const std::vector<size_t>& dims, size_t current_dim,
                         std::vector<typename type_of_array<T>::type>& buffer) {

    check_dimensions_vector(vec_multi_dim.size(), dims[current_dim],
                            current_dim);
    for (typename std::vector<T>::const_iterator it = vec_multi_dim.begin();
         it < vec_multi_dim.end(); ++it) {
        vectors_to_single_buffer(*it, dims, current_dim + 1, buffer);
    }
}

// copy single buffer to multi dimensional vector, following dimensions
// specified
template <typename T>
inline typename std::vector<T>::iterator
single_buffer_to_vectors(typename std::vector<T>::iterator begin_buffer,
                         typename std::vector<T>::iterator end_buffer,
                         const std::vector<size_t>& dims, size_t current_dim,
                         std::vector<T>& vec_single_dim) {
    const size_t n_elems = dims[current_dim];
    typename std::vector<T>::iterator end_copy_iter =
        std::min(begin_buffer + n_elems, end_buffer);
    vec_single_dim.assign(begin_buffer, end_copy_iter);
    return end_copy_iter;
}

template <typename T, typename U>
inline typename std::vector<T>::iterator
single_buffer_to_vectors(typename std::vector<T>::iterator begin_buffer,
                         typename std::vector<T>::iterator end_buffer,
                         const std::vector<size_t>& dims, size_t current_dim,
                         std::vector<U>& vec_multi_dim) {

    const size_t n_elems = dims[current_dim];
    vec_multi_dim.resize(n_elems);

    for (typename std::vector<U>::iterator it = vec_multi_dim.begin();
         it < vec_multi_dim.end(); ++it) {
        begin_buffer = single_buffer_to_vectors(begin_buffer, end_buffer, dims,
                                                current_dim + 1, *it);
    }
    return begin_buffer;
}

// apply conversion operations to basic scalar type
template <typename Scalar, class Enable = void>
struct data_converter {
    inline data_converter(Scalar &datamem, DataSpace &space, const bool doTranspose = false) {
        static_assert((std::is_arithmetic<Scalar>::value ||
                       std::is_same<std::string, Scalar>::value),
                      "supported datatype should be an arithmetic value, a "
                      "std::string or a container/array");
        (void)datamem;
        (void)space; // do nothing
    }

    inline Scalar* transform_read(Scalar& datamem) { return &datamem; }

    inline Scalar* transform_write(Scalar& datamem) { return &datamem; }

    inline void process_result(Scalar& datamem) {
        (void)datamem; // do nothing
    }
};

// apply conversion operations to the incoming data
// if they are a cstyle array
template <typename CArray>
struct data_converter<CArray,
                      typename enable_if<(is_c_array<CArray>::value)>::type> {
    inline data_converter(CArray &datamem, DataSpace &space, const bool doTranspose = false) {
        (void)datamem;
        (void)space; // do nothing
    }

    inline CArray& transform_read(CArray& datamem) { return datamem; }

    inline CArray& transform_write(CArray& datamem) { return datamem; }

    inline void process_result(CArray& datamem) {
        (void)datamem; // do nothing
    }
};

// apply conversion for vectors 1D
  template <typename T>
  struct data_converter<
    std::vector<T>,
    typename enable_if<(is_same<T, typename type_of_array<T>::type>::value)>::type> {
      inline data_converter(std::vector<T> &vec, DataSpace &space, size_t dim = 0, const bool doTranspose = false)
              : _space(
              &space), _dim(dim) {
      assert(_space->getDimensions().size() > dim);
      (void)vec;
    }

    inline typename type_of_array<T>::type*
    transform_read(std::vector<T>& vec) {
      vec.resize(_space->getDimensions()[_dim]);
      return &(vec[0]);
    }

    inline typename type_of_array<T>::type*
    transform_write(std::vector<T>& vec) {
      return &(vec[0]);
    }

    inline void process_result(std::vector<T>& vec) { (void)vec; }

    DataSpace* _space;
    size_t _dim;
  };

#ifdef H5_USE_EIGEN

// apply conversion to eigen matrix
        template<typename Scalar, int RowsAtCompileTime, int ColsAtCompileTime, int Options>
        struct data_converter<Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime, Options>, void> {
            typedef Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime, Options> Matrix;

            inline data_converter(Matrix &matrix, DataSpace &space, const bool doTranspose = false, size_t dim = 0)
                    : disk_dims(space.getDimensions()),
                      mem_dims(disk_dims),
                      is_row_major{Options == Eigen::RowMajor},
                      transposed(doTranspose) {
                if (transposed) {
                    if (is_row_major) {
                        _temp_col_matrix = matrix;
                    }
                } else {
                    if (!is_row_major) {
                        _temp_row_matrix = matrix;
                    }
                }
                assert(disk_dims.size() == 2);
                (void)
                        dim;
                (void)
                        matrix;
            }

            inline typename type_of_array<Scalar>::type *transform_read(Matrix &matrix) {

                auto return_pointer = matrix.data();
                if (std::pair<size_t, size_t>(mem_dims[0], mem_dims[1]) !=
                    std::pair<size_t, size_t>(matrix.rows(), matrix.cols())) {
                    matrix.resize(mem_dims[0], mem_dims[1]);
                }
                return_pointer = matrix.data();
                if (transposed) {
                    if (is_row_major) {
                        _temp_col_matrix.resize(mem_dims[0], mem_dims[1]);
                        return_pointer = _temp_col_matrix.data();
                    }
                } else {
                    if (!is_row_major) {
                        _temp_row_matrix.resize(mem_dims[0], mem_dims[1]);
                        return_pointer = _temp_row_matrix.data();
                    }
                }
                return return_pointer;

            }

            inline typename type_of_array<Scalar>::type *transform_write(Matrix &matrix) {

                auto return_pointer = matrix.data();
                if (transposed) {
                    if (is_row_major) {
                        _temp_col_matrix.resize(mem_dims[0], mem_dims[1]);
                        _temp_col_matrix = matrix;
                        return_pointer = _temp_col_matrix.data();
                    }
                } else {
                    if (!is_row_major) {
                        _temp_row_matrix.resize(mem_dims[0], mem_dims[1]);
                        _temp_row_matrix = matrix;
                        return_pointer = _temp_row_matrix.data();
                    }
                }
                return return_pointer;
            }

            inline void process_result(Matrix &matrix) {
                if (transposed) {
                    if (is_row_major) {
                        matrix = _temp_col_matrix;
                    }
                } else {
                    if (!is_row_major) {
                        matrix = _temp_row_matrix;
                    }
                }

                (void) matrix;
            }

            std::vector<size_t> disk_dims;
            std::vector<size_t> mem_dims;
            const bool is_row_major;
            const bool transposed;
            Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime, Eigen::RowMajor> _temp_row_matrix;
            Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime, Eigen::ColMajor> _temp_col_matrix;

};


// apply conversion to eigen matrix
        template<typename Scalar, int RowsAtCompileTime, int ColsAtCompileTime, int Options>
        struct data_converter<Eigen::Map<Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime, Options>>, void> {
            typedef Eigen::Map<Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime, Options>> Matrix;

            inline data_converter(Matrix &matrix, DataSpace &space, const bool doTranspose = false, size_t dim = 0)
                    : disk_dims(space.getDimensions()),
                      mem_dims(disk_dims),
                      is_row_major{Options == Eigen::RowMajor},
                      transposed(doTranspose) {
                if (transposed) {
                    std::reverse(mem_dims.begin(), mem_dims.end());
                    if (is_row_major) {
                        _temp_col_matrix = matrix;
                    }
                } else {
                    if (!is_row_major) {
                        _temp_row_matrix = matrix;
                    }
                }
                assert(disk_dims.size() == 2);
                (void)
                        dim;
                (void)
                        matrix;
            }


            inline typename type_of_array<Scalar>::type *transform_read(Matrix &matrix) {

                assert(mem_dims[0] == (unsigned long) matrix.rows());
                assert(mem_dims[1] == (unsigned long) matrix.cols());
                auto return_pointer = matrix.data();

                if (transposed) {
                    if (is_row_major) {
                        _temp_col_matrix.resize(mem_dims[0], mem_dims[1]);
                        return_pointer = _temp_col_matrix.data();
                    }
                } else {
                    if (!is_row_major) {
                        _temp_row_matrix.resize(mem_dims[0], mem_dims[1]);
                        return_pointer = _temp_row_matrix.data();
                    }
                }
                return return_pointer;

            }

            inline typename type_of_array<Scalar>::type *transform_write(Matrix &matrix) {
                auto return_pointer = matrix.data();
                if (transposed) {
                    if (is_row_major) {
                        _temp_col_matrix.resize(mem_dims[0], mem_dims[1]);
                        _temp_col_matrix = matrix;
                        return_pointer = _temp_col_matrix.data();
                    }
                } else {
                    if (!is_row_major) {
                        _temp_row_matrix.resize(mem_dims[0], mem_dims[1]);
                        _temp_row_matrix = matrix;
                        return_pointer = _temp_row_matrix.data();
                    }
                }
                return return_pointer;

            }

            inline void process_result(Matrix &matrix) {

                if (transposed) {
                    if (is_row_major) {
                        matrix = _temp_col_matrix;
                    }
                } else {
                    if (!is_row_major) {
                        matrix = _temp_row_matrix;
                    }
                }
                (void) matrix;
            }

            std::vector<size_t> disk_dims;
            std::vector<size_t> mem_dims;
            const bool is_row_major;
            const bool transposed;
            Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime, Eigen::RowMajor> _temp_row_matrix;
            Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime, Eigen::ColMajor> _temp_col_matrix;

        };


#endif





#ifdef H5_USE_BOOST
// apply conversion to boost multi array
template <typename T, std::size_t Dims>
struct data_converter<boost::multi_array<T, Dims>, void> {
    typedef typename boost::multi_array<T, Dims> MultiArray;


    inline data_converter(MultiArray &array, DataSpace &space, size_t dim = 0, const bool doTranspose = false)
            : _dims(space.getDimensions()),
              is_row_major(array.storage_order() == boost::c_storage_order()) {
        if (!is_row_major) {
            _temp_array = array;
        }
        assert(_dims.size() == Dims);
        (void)dim;
        (void)array;
    }

    inline typename type_of_array<T>::type* transform_read(MultiArray& array) {

        auto return_pointer = array.data();
        if (std::equal(_dims.begin(), _dims.end(), array.shape()) == false) {
            boost::array<typename MultiArray::index, Dims> ext;
            std::copy(_dims.begin(), _dims.end(), ext.begin());
            array.resize(ext);
            return_pointer = array.data();

        }
        if (!is_row_major) {
            boost::array<typename MultiArray::index, Dims> ext;
            std::copy(_dims.begin(), _dims.end(), ext.begin());
            _temp_array.resize(ext);
            return_pointer = _temp_array.data();
        }
        return return_pointer;
    }

    inline typename type_of_array<T>::type* transform_write(MultiArray& array) {
        assert(array.storage_order() == boost::c_storage_order());
        return array.data();
    }

    inline void process_result(MultiArray &array) {
        if (!(array.storage_order() == boost::c_storage_order())) {
            MultiArray temp_array(_dims, boost::c_storage_order());
            boost::detail::multi_array::copy_n(array.begin(), array.num_elements(), temp_array.begin());
            array = temp_array;
        }

        (void) array;
    }

    std::vector<size_t> _dims;
    const bool is_row_major;
    boost::multi_array<T, Dims> _temp_array;

};

// apply conversion to boost matrix ublas
        template<typename T, class L, class A>
        struct data_converter<boost::numeric::ublas::matrix<T, L, A>, void> {

            typedef typename boost::numeric::ublas::matrix<T, L, A> Matrix;
            typedef typename boost::numeric::ublas::matrix<T, boost::numeric::ublas::row_major> Matrix_rm;

            inline data_converter(Matrix &array, DataSpace &space, size_t dim = 0, const bool doTranspose = false)
                    : _dims(space.getDimensions()),
                      is_row_major(
                              std::is_same<
                                      typename boost::numeric::ublas::matrix<T, L, A>::orientation_category,
                                      typename Matrix_rm::orientation_category
                              >::value
                      ) {
                assert(_dims.size() == 2);
                (void) dim;
                (void) array;
            }

            inline typename type_of_array<T>::type *transform_read(Matrix &array) {
                boost::array<std::size_t, 2> sizes = {{array.size1(), array.size2()}};

                if (std::equal(_dims.begin(), _dims.end(), sizes.begin()) == false) {
                    array.resize(_dims[0], _dims[1], false);
                    array(0, 0) = 0; // force initialization
                }
                auto return_pointer = &(array(0, 0));
                if (!is_row_major) {
                    _temp_array.resize(_dims[0], _dims[1]);
                    _temp_array(0, 0) = 0;
                    return_pointer = &(_temp_array(0, 0));
                }

                return return_pointer;
            }

            inline typename type_of_array<T>::type *transform_write(Matrix &array) {
                auto return_pointer = &(array(0, 0));
                if (!is_row_major) {
                    _temp_array.resize(_dims[0], _dims[1]);
                    _temp_array(0, 0) = 0;
                    _temp_array = array;
                    return_pointer = &(_temp_array(0, 0));

                }
                return return_pointer;
            }

            inline void process_result(Matrix &array) {

                if (!is_row_major) {
                    array = _temp_array;
                }

                (void) array;
            }


            std::vector<size_t> _dims;
            const bool is_row_major;
            Matrix_rm _temp_array;
        };



#endif

// apply conversion for vectors nested vectors
template <typename T>
struct data_converter<std::vector<T>,
                      typename enable_if<(is_container<T>::value)>::type> {
    inline data_converter(std::vector<T> &vec, DataSpace &space, size_t dim = 0, const bool doTranspose = false)
        : _dims(space.getDimensions()), _dim(dim), _vec_align() {
        (void)vec;
    }

    inline typename type_of_array<T>::type*
    transform_read(std::vector<T>& vec) {
        (void)vec;
        _vec_align.resize(get_total_size());
        return &(_vec_align[0]);
    }

    inline typename type_of_array<T>::type*
    transform_write(std::vector<T>& vec) {
        _vec_align.reserve(get_total_size());
        vectors_to_single_buffer<T>(vec, _dims, 0, _vec_align);
        return &(_vec_align[0]);
    }

    inline void process_result(std::vector<T>& vec) {
        single_buffer_to_vectors<typename type_of_array<T>::type, T>(
            _vec_align.begin(), _vec_align.end(), _dims, 0, vec);
    }

    inline size_t get_total_size() {
        return std::accumulate(_dims.begin(), _dims.end(), 1,
                               std::multiplies<size_t>());
    }

    std::vector<size_t> _dims;
    size_t _dim;
    std::vector<typename type_of_array<T>::type> _vec_align;
};

// apply conversion to scalar string
template <>
struct data_converter<std::string, void> {
    inline data_converter(std::string &vec, DataSpace &space, const bool doTranspose = false) : _space(space) {
        (void)vec;
    }

    // create a C vector adapted to HDF5
    // fill last element with NULL to identify end
    inline char *transform_read(std::string &) {
        _c_vec.resize(255);

        return (_c_vec.data());
    }

    static inline char* char_converter(const std::string& str) {
        return const_cast<char*>(str.c_str());
    }

    inline char *transform_write(std::string &str) {
        _c_vec.resize(255);
        std::copy(str.begin(), str.end(), _c_vec.begin());
        return _c_vec.data();
    }

    inline void process_result(std::string& str) {

        str = std::string(&_c_vec[0]);

    }

    std::vector<char> _c_vec;
    DataSpace& _space;
};

        template<>
        struct data_converter<std::vector<std::string>, void> {
            inline data_converter(std::vector<std::string> &vec, DataSpace &space, const bool doTranspose = false)
                    : _space(space) {
                (void) vec;
            }

            // create a C vector adapted to HDF5
            // fill last element with NULL to identify end
            inline char *transform_read(std::vector<std::string> &vec) {
                (void) vec;
                _c_vec.resize(255 * _space.getDimensions()[0]);

                //_c_vec.resize(_space.getDimensions()[0], NULL);
                return (_c_vec.data());
            }

            static inline char *char_converter(const std::string &str) {
                return const_cast<char *>(str.c_str());
            }

            inline char *transform_write(std::vector<std::string> &vec) {
                _c_vec.resize(255 * _space.getDimensions()[0]);
                std::vector<char>::iterator it = _c_vec.begin();
                const size_t vs = vec.size();
                for (size_t i = 0; i < vs; i++) {
                    auto tb = vec[i].begin();
                    auto te = vec[i].end();
                    std::copy(tb, te, it);
                    it += 255;
                }

                return (_c_vec.data());
            }

            inline void process_result(std::vector<std::string> &vec) {
                (void) vec;
                const size_t vs = _space.getDimensions()[0];
                vec.resize(vs);
                for (size_t i = 0; i < vs; ++i) {
                    vec[i] = std::string(&_c_vec[i * 255]);
                }
                /*   if (_c_vec.empty() == false && _c_vec[0] != NULL) {
                    AtomicType<std::string> str_type;
                    (void) H5Dvlen_reclaim(str_type.getId(), _space.getId(), H5P_DEFAULT,
                                           &(_c_vec[0]));
                }*/
            }

            std::vector<char> _c_vec;
            DataSpace &_space;
        };
    }
}

#endif // H5CONVERTER_MISC_HPP
