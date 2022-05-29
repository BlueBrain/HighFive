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

// DATA CONVERTERS
// ===============

// apply conversion operations to basic scalar type
template <typename T, class Enable>
struct data_converter {
    using hdf5_type = typename inspector<T>::hdf5_type;
    inline data_converter(const DataSpace& space)
        : _dims(space.getDimensions())
        , _space(space) {}

    inline hdf5_type* transform_read(T&) {
        _vec_align.resize(compute_total_size(_dims));
        return _vec_align.data();
    }

    inline const hdf5_type* transform_write(const T& datamem) {
        _vec_align = inspector<T>::serialize(datamem);
        return _vec_align.data();
    }

    inline void process_result(T& val) {
        inspector<T>::prepare(val, _dims);
        val = inspector<T>::unserialize(_vec_align.data(), _dims);
        auto t = create_datatype<typename inspector<T>::base_type>();
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


// apply conversion for fixed-string. Implements container interface
template <std::size_t N>
struct data_converter<FixedLenStringArray<N>, void>
    : public container_converter<FixedLenStringArray<N>, char> {
    using container_converter<FixedLenStringArray<N>, char>::container_converter;
};

}  // namespace details

}  // namespace HighFive

#endif  // H5CONVERTER_MISC_HPP
