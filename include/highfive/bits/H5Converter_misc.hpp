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
#include <boost/numeric/ublas/matrix.hpp>
#endif

#include <H5Dpublic.h>
#include <H5Ppublic.h>

#include "../H5Reference.hpp"
#include "H5Utils.hpp"

namespace HighFive {

namespace details {

inline bool is_1D(const std::vector<size_t>& dims) {
    return std::count_if(dims.begin(), dims.end(), [](size_t i) { return i > 1; }) < 2;
}

inline size_t compute_total_size(const std::vector<size_t>& dims) {
    return std::accumulate(dims.begin(), dims.end(), size_t{1u}, std::multiplies<size_t>());
}

inline void check_dimensions_vector(size_t size_vec, size_t size_dataset, size_t dimension) {
    if (size_vec != size_dataset) {
        std::ostringstream ss;
        ss << "Mismatch between vector size (" << size_vec << ") and dataset size ("
           << size_dataset;
        ss << ") on dimension " << dimension;
        throw DataSetException(ss.str());
    }
}


// DATA CONVERTERS
// ===============

// apply conversion operations to basic scalar type
template <typename Scalar, class Enable>
struct data_converter {
    using hdf5_type = typename inspector<Scalar>::hdf5_type;
    inline data_converter(const DataSpace& space)
        : _dims(space.getDimensions())
        , _space(space) {}

    inline hdf5_type* transform_read(Scalar&) {
        _vec_align.resize(compute_total_size(_dims));
        return _vec_align.data();
    }

    inline const hdf5_type* transform_write(const Scalar& datamem) {
        _vec_align = inspector<Scalar>::serialize(datamem);
        return _vec_align.data();
    }

    inline void process_result(Scalar& val) {
        val = inspector<Scalar>::unserialize(_vec_align.data(), _dims);
        auto t = create_datatype<typename inspector<Scalar>::base_type>();
        auto c = t.getClass();
        if (c == DataTypeClass::VarLen) {
            (void) H5Dvlen_reclaim(t.getId(), _space.getId(), H5P_DEFAULT, &val);
        }
    }

    std::vector<size_t> _dims;
    DataSpace _space;
    std::vector<hdf5_type> _vec_align;
};


// apply conversion operations to the incoming data
// if they are a cstyle array
template <typename CArray>
struct data_converter<CArray, typename std::enable_if<(is_c_array<CArray>::value)>::type> {
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
    using value_type = T;

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


// apply conversion to std::array
template <typename T, std::size_t S>
struct data_converter<
    std::array<T, S>,
    typename std::enable_if<(std::is_same<T, typename inspector<T>::base_type>::value)>::type>
    : public container_converter<std::array<T, S>> {
    inline data_converter(const DataSpace& space)
        : container_converter<std::array<T, S>>(space) {
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
// apply conversion to boost matrix ublas
template <typename T>
struct data_converter<boost::numeric::ublas::matrix<T>, void>
    : public container_converter<boost::numeric::ublas::matrix<T>> {
    using Matrix = boost::numeric::ublas::matrix<T>;
    using value_type = typename inspector<T>::base_type;

    inline data_converter(const DataSpace& space)
        : container_converter<Matrix>(space) {
        assert(space.getDimensions().size() == 2);
    }

    inline value_type* transform_read(Matrix& array) {
        boost::array<std::size_t, 2> sizes = {{array.size1(), array.size2()}};
        auto&& _dims = this->_space.getDimensions();
        if (std::equal(_dims.begin(), _dims.end(), sizes.begin()) == false) {
            array.resize(_dims[0], _dims[1], false);
            array(0, 0) = 0;  // force initialization
        }
        return &(array(0, 0));
    }

    inline const value_type* transform_write(const Matrix& array) const noexcept {
        return &(array(0, 0));
    }
};
#endif


// apply conversion for fixed-string. Implements container interface
template <std::size_t N>
struct data_converter<FixedLenStringArray<N>, void>
    : public container_converter<FixedLenStringArray<N>, char> {
    using container_converter<FixedLenStringArray<N>, char>::container_converter;
};

}  // namespace details

}  // namespace HighFive

#endif  // H5CONVERTER_MISC_HPP
