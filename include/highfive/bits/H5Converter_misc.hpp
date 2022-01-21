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
#include <array>

#ifdef H5_USE_BOOST
// starting Boost 1.64, serialization header must come before ublas
#include <boost/serialization/vector.hpp>
#include <boost/multi_array.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#endif

#include <H5Dpublic.h>
#include <H5Ppublic.h>

#include "../H5Reference.hpp"
#include "H5Utils.hpp"

namespace HighFive {

namespace details {

inline bool is_1D(const std::vector<size_t>& dims) {
    return std::count_if(dims.begin(), dims.end(), [](size_t i){ return i > 1; }) < 2;
}

inline size_t compute_total_size(const std::vector<size_t>& dims) {
    return std::accumulate(dims.begin(), dims.end(), size_t{1u},
                           std::multiplies<size_t>());
}

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


// Buffer converters
// =================

// copy multi dimensional vector in C++ in one C-style multi dimensional buffer
template <typename T>
inline void vectors_to_single_buffer(const std::vector<T>& vec_single_dim,
                                     const std::vector<size_t>& dims,
                                     const size_t current_dim,
                                     std::vector<T>& buffer) {

    check_dimensions_vector(vec_single_dim.size(), dims[current_dim], current_dim);
    buffer.insert(buffer.end(), vec_single_dim.begin(), vec_single_dim.end());
}


template <typename T, typename U = typename inspector<T>::base_type>
inline void
vectors_to_single_buffer(const std::vector<T>& vec_multi_dim,
                         const std::vector<size_t>& dims,
                         size_t current_dim,
                         std::vector<U>& buffer) {

    check_dimensions_vector(vec_multi_dim.size(), dims[current_dim], current_dim);
    for (const auto& it : vec_multi_dim) {
        vectors_to_single_buffer(it, dims, current_dim + 1, buffer);
    }
}

// copy single buffer to multi dimensional vector, following specified dimensions
template <typename T>
inline typename std::vector<T>::const_iterator
single_buffer_to_vectors(typename std::vector<T>::const_iterator begin_buffer,
                         typename std::vector<T>::const_iterator end_buffer,
                         const std::vector<size_t>& dims,
                         const size_t current_dim,
                         std::vector<T>& vec_single_dim) {
    const auto n_elems = static_cast<long>(dims[current_dim]);
    const auto end_copy_iter = std::min(begin_buffer + n_elems, end_buffer);
    vec_single_dim.assign(begin_buffer, end_copy_iter);
    return end_copy_iter;
}

template <typename T, typename U = typename inspector<T>::base_type>
inline typename std::vector<U>::const_iterator
single_buffer_to_vectors(typename std::vector<U>::const_iterator begin_buffer,
                         typename std::vector<U>::const_iterator end_buffer,
                         const std::vector<size_t>& dims,
                         const size_t current_dim,
                         std::vector<std::vector<T>>& vec_multi_dim) {
    const size_t n_elems = dims[current_dim];
    vec_multi_dim.resize(n_elems);

    for (auto& subvec : vec_multi_dim) {
        begin_buffer = single_buffer_to_vectors(
            begin_buffer, end_buffer, dims, current_dim + 1, subvec);
    }
    return begin_buffer;
}


// DATA CONVERTERS
// ===============

// apply conversion operations to basic scalar type
template <typename Scalar, class Enable>
struct data_converter {
    inline data_converter(const DataSpace&) noexcept {

        static_assert((std::is_arithmetic<Scalar>::value ||
                       std::is_enum<Scalar>::value ||
                       std::is_same<std::string, Scalar>::value),
                      "supported datatype should be an arithmetic value, a "
                      "std::string or a container/array");
    }

    inline Scalar* transform_read(Scalar& datamem) const noexcept {
        return &datamem;
    }

    inline const Scalar* transform_write(const Scalar& datamem) const noexcept {
        return &datamem;
    }

    inline void process_result(Scalar&) const noexcept {}
};


// apply conversion operations to the incoming data
// if they are a cstyle array
template <typename CArray>
struct data_converter<CArray,
                      typename std::enable_if<(is_c_array<CArray>::value)>::type> {
    inline data_converter(const DataSpace&) noexcept {}

    inline CArray& transform_read(CArray& datamem) const noexcept {
        return datamem;
    }

    inline const CArray& transform_write(const CArray& datamem) const noexcept {
        return datamem;
    }

    inline void process_result(CArray&) const noexcept {}
};

// Generic container converter
template <typename Container, typename T = typename inspector<Container>::base_type>
struct container_converter {
    typedef T value_type;

    inline container_converter(const DataSpace& space)
        : _space(space) {}

    // Ship (pseudo)1D implementation
    inline value_type* transform_read(Container& vec) const {
        auto&& dims = _space.getDimensions();
        if (!is_1D(dims))
            throw DataSpaceException("Dataset cant be converted to 1D");
        vec.resize(compute_total_size(dims));
        return vec.data();
    }

    inline const value_type* transform_write(const Container& vec) const noexcept {
        return vec.data();
    }

    inline void process_result(Container&) const noexcept {}

    const DataSpace& _space;
};


// apply conversion for continuous vectors
template <typename T>
struct data_converter<
    std::vector<T>,
    typename std::enable_if<std::is_trivially_copyable<T>::value>::type>
    : public container_converter<std::vector<T>> {

    using container_converter<std::vector<T>>::container_converter;
};


// apply conversion to std::array
template <typename T, std::size_t S>
struct data_converter<
    std::array<T, S>,
    typename std::enable_if<(
        std::is_same<T, typename inspector<T>::base_type>::value)>::type>
    : public container_converter<std::array<T, S>> {

    inline data_converter(const DataSpace& space)
        : container_converter<std::array<T, S>>(space)
    {
        auto&& dims = space.getDimensions();
        if (!is_1D(dims)) {
            throw DataSpaceException("Only 1D std::array supported currently.");
        }
        if (compute_total_size(dims) != S) {
            std::ostringstream ss;
            ss << "Impossible to pair DataSet with " << compute_total_size(dims)
               << " elements into an array with " << S << " elements.";
            throw DataSpaceException(ss.str());
        }
    }

    inline T* transform_read(std::array<T, S>& vec) const noexcept {
        return vec.data();
    }
};


#ifdef H5_USE_BOOST
// apply conversion to boost multi array
template <typename T, std::size_t Dims>
struct data_converter<boost::multi_array<T, Dims>, void>
    : public container_converter<boost::multi_array<T, Dims>> {
    using MultiArray = boost::multi_array<T, Dims>;
    using value_type = typename inspector<T>::base_type;
    using container_converter<MultiArray>::container_converter;

    inline value_type* transform_read(MultiArray& array) {
        auto&& dims = this->_space.getDimensions();
        if (std::equal(dims.begin(), dims.end(), array.shape()) == false) {
            boost::array<typename MultiArray::index, Dims> ext;
            std::copy(dims.begin(), dims.end(), ext.begin());
            array.resize(ext);
        }
        return array.data();
    }
};


// apply conversion to boost matrix ublas
template <typename T>
struct data_converter<boost::numeric::ublas::matrix<T>, void>
    : public container_converter<boost::numeric::ublas::matrix<T>> {
    using Matrix = boost::numeric::ublas::matrix<T>;
    using value_type = typename inspector<T>::base_type;

    inline data_converter(const DataSpace& space) : container_converter<Matrix>(space) {
        assert(space.getDimensions().size() == 2);
    }

    inline value_type* transform_read(Matrix& array) {
        boost::array<std::size_t, 2> sizes = {{array.size1(), array.size2()}};
        auto&& _dims = this->_space.getDimensions();
        if (std::equal(_dims.begin(), _dims.end(), sizes.begin()) == false) {
            array.resize(_dims[0], _dims[1], false);
            array(0, 0) = 0; // force initialization
        }
        return &(array(0, 0));
    }

    inline const value_type* transform_write(const Matrix& array) const noexcept {
        return &(array(0, 0));
    }
};
#endif


// apply conversion for vectors nested vectors
template <typename T>
struct data_converter<std::vector<T>,
                      typename std::enable_if<(is_container<T>::value)>::type> {
    using value_type = typename inspector<T>::base_type;

    inline data_converter(const DataSpace& space)
        : _dims(space.getDimensions()) {}

    inline value_type* transform_read(std::vector<T>&) {
        _vec_align.resize(compute_total_size(_dims));
        return _vec_align.data();
    }

    inline const value_type* transform_write(const std::vector<T>& vec) {
        _vec_align.reserve(compute_total_size(_dims));
        vectors_to_single_buffer<T>(vec, _dims, 0, _vec_align);
        return _vec_align.data();
    }

    inline void process_result(std::vector<T>& vec) const {
        single_buffer_to_vectors(
            _vec_align.cbegin(), _vec_align.cend(), _dims, 0, vec);
    }

    std::vector<size_t> _dims;
    std::vector<typename inspector<T>::base_type> _vec_align;
};


// apply conversion to scalar string
template <>
struct data_converter<std::string, void> {
    using value_type = const char*;  // char data is const, mutable pointer

    inline data_converter(const DataSpace& space) noexcept
        : _c_vec(nullptr)
        , _space(space) {}

    // create a C vector adapted to HDF5
    // fill last element with NULL to identify end
    inline value_type* transform_read(std::string&) noexcept {
        return &_c_vec;
    }

    inline const value_type* transform_write(const std::string& str) noexcept {
        _c_vec = str.c_str();
        return &_c_vec;
    }

    inline void process_result(std::string& str) {
        assert(_c_vec != nullptr);
        str = std::string(_c_vec);

        if (_c_vec != nullptr) {
            AtomicType<std::string> str_type;
            (void)H5Dvlen_reclaim(str_type.getId(), _space.getId(), H5P_DEFAULT,
                                  &_c_vec);
        }
    }

    value_type _c_vec;
    const DataSpace& _space;
};

// apply conversion for vectors of string (dereference)
template <>
struct data_converter<std::vector<std::string>, void> {
    using value_type = const char*;

    inline data_converter(const DataSpace& space) noexcept
        : _space(space) {}

    // create a C vector adapted to HDF5
    // fill last element with NULL to identify end
    inline value_type* transform_read(std::vector<std::string>&) {
        _c_vec.resize(_space.getDimensions()[0], NULL);
        return _c_vec.data();
    }

    inline const value_type* transform_write(const std::vector<std::string>& vec) {
        _c_vec.resize(vec.size() + 1, NULL);
        std::transform(vec.begin(), vec.end(), _c_vec.begin(),
                       [](const std::string& str){ return str.c_str(); });
        return _c_vec.data();
    }

    inline void process_result(std::vector<std::string>& vec) {
        vec.resize(_c_vec.size());
        for (size_t i = 0; i < vec.size(); ++i) {
            vec[i] = std::string(_c_vec[i]);
        }

        if (_c_vec.empty() == false && _c_vec[0] != NULL) {
            AtomicType<std::string> str_type;
            (void)H5Dvlen_reclaim(str_type.getId(), _space.getId(), H5P_DEFAULT,
                                  &(_c_vec[0]));
        }
    }

    std::vector<value_type> _c_vec;
    const DataSpace& _space;
};



// apply conversion for fixed-string. Implements container interface
template <std::size_t N>
struct data_converter<FixedLenStringArray<N>, void>
    : public container_converter<FixedLenStringArray<N>, char> {
    using container_converter<FixedLenStringArray<N>, char>::container_converter;
};

template <>
struct data_converter<std::vector<Reference>, void> {
    inline data_converter(const DataSpace& space)
        : _dims(space.getDimensions()) {
        if (!is_1D(_dims)) {
            throw DataSpaceException("Only 1D std::array supported currently.");
        }
    }

    inline hobj_ref_t* transform_read(std::vector<Reference>& vec) {
        auto total_size = compute_total_size(_dims);
        _vec_align.resize(total_size);
        vec.resize(total_size);
        return _vec_align.data();
    }

    inline const hobj_ref_t* transform_write(const std::vector<Reference>& vec) {
        _vec_align.resize(compute_total_size(_dims));
        for (size_t i = 0; i < vec.size(); ++i) {
            vec[i].create_ref(&_vec_align[i]);
        }
        return _vec_align.data();
    }

    inline void process_result(std::vector<Reference>& vec) const {
        auto* href = const_cast<hobj_ref_t*>(_vec_align.data());
        for (auto& ref : vec) {
            ref = Reference(*(href++));
        }
    }

    std::vector<size_t> _dims;
    std::vector<typename inspector<hobj_ref_t>::base_type> _vec_align;
};

}  // namespace details

}  // namespace HighFive

#ifdef H5_USE_EIGEN
#include "H5ConverterEigen_misc.hpp"
#endif

#endif // H5CONVERTER_MISC_HPP
