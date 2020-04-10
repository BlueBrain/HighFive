/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5DATASPACE_MISC_HPP
#define H5DATASPACE_MISC_HPP

#include <array>
#include <initializer_list>
#include <vector>
#include <numeric>

#include <H5Spublic.h>

#include "H5Utils.hpp"

namespace HighFive {

inline DataSpace::DataSpace(const std::vector<size_t>& dims)
    : DataSpace(dims.begin(), dims.end()) {}

inline DataSpace::DataSpace(const std::initializer_list<size_t>& items)
    : DataSpace(std::vector<size_t>(items)) {}

template<typename... Args>
    inline DataSpace::DataSpace(size_t dim1, Args... dims)
    : DataSpace(std::vector<size_t>{dim1, static_cast<size_t>(dims)...}) {}

template <class IT, typename>
inline DataSpace::DataSpace(const IT begin, const IT end) {
    std::vector<hsize_t> real_dims(begin, end);

    if ((_hid = H5Screate_simple(int(real_dims.size()), real_dims.data(),
                                 NULL)) < 0) {
        throw DataSpaceException("Impossible to create dataspace");
    }
}

inline DataSpace::DataSpace(const std::vector<size_t>& dims,
                            const std::vector<size_t>& maxdims) {

    if (dims.size() != maxdims.size()) {
        throw DataSpaceException("dims and maxdims must be the same length.");
    }

    std::vector<hsize_t> real_dims(dims.begin(), dims.end());
    std::vector<hsize_t> real_maxdims(maxdims.begin(), maxdims.end());

    // Replace unlimited flag with actual HDF one
    std::replace(real_maxdims.begin(), real_maxdims.end(),
                 static_cast<hsize_t>(DataSpace::UNLIMITED), H5S_UNLIMITED);

    if ((_hid = H5Screate_simple(int(dims.size()), real_dims.data(),
                                 real_maxdims.data())) < 0) {
        throw DataSpaceException("Impossible to create dataspace");
    }
} // namespace HighFive

inline DataSpace::DataSpace(DataSpace::DataspaceType dtype) {
    H5S_class_t h5_dataspace_type;
    switch (dtype) {
    case DataSpace::datascape_scalar:
        h5_dataspace_type = H5S_SCALAR;
        break;
    case DataSpace::datascape_null:
        h5_dataspace_type = H5S_NULL;
        break;
    default:
        throw DataSpaceException("Invalid dataspace type: should be "
                                 "datascape_scalar or datascape_null");
    }

    if ((_hid = H5Screate(h5_dataspace_type)) < 0) {
        throw DataSpaceException("Unable to create dataspace");
    }
}

inline DataSpace DataSpace::clone() const {
    DataSpace res;
    if ((res._hid = H5Scopy(_hid)) < 0) {
        throw DataSpaceException("Unable to copy dataspace");
    }
    return res;
}

inline size_t DataSpace::getNumberDimensions() const {
    const int ndim = H5Sget_simple_extent_ndims(_hid);
    if (ndim < 0) {
        HDF5ErrMapper::ToException<DataSetException>(
            "Unable to get dataspace number of dimensions");
    }
    return size_t(ndim);
}

inline std::vector<size_t> DataSpace::getDimensions() const {
    std::vector<hsize_t> dims(getNumberDimensions());
    if (!dims.empty()) {
        if (H5Sget_simple_extent_dims(_hid, dims.data(), NULL) < 0) {
            HDF5ErrMapper::ToException<DataSetException>(
                "Unable to get dataspace dimensions");
        }
    }
    return details::to_vector_size_t(std::move(dims));
}

inline size_t DataSpace::getElementCount() const {
    const std::vector<size_t>& dims = getDimensions();
    return std::accumulate(dims.begin(), dims.end(), size_t{1u},
                           std::multiplies<size_t>());
}

inline std::vector<size_t> DataSpace::getMaxDimensions() const {
    std::vector<hsize_t> maxdims(getNumberDimensions());
    if (H5Sget_simple_extent_dims(_hid, NULL, maxdims.data()) < 0) {
        HDF5ErrMapper::ToException<DataSetException>(
            "Unable to get dataspace dimensions");
    }

    std::replace(maxdims.begin(), maxdims.end(), H5S_UNLIMITED,
                 static_cast<hsize_t>(DataSpace::UNLIMITED));
    return details::to_vector_size_t(maxdims);
}

template <typename ScalarValue>
inline DataSpace DataSpace::From(const ScalarValue& scalar) {
    (void)scalar;
    static_assert(
        (std::is_arithmetic<ScalarValue>::value ||
         std::is_enum<ScalarValue>::value ||
         std::is_same<std::string, ScalarValue>::value),
        "Only the following types are supported by DataSpace::From: \n"
        "  signed_arithmetic_types = int | long | float | double \n"
        "  unsigned_arithmetic_types = unsigned signed_arithmetic_types \n"
        "  string_types = std::string \n"
        "  all_basic_types = string_types | unsigned_arithmetic_types | "
        "signed_arithmetic_types \n "
        "  stl_container_types = std::vector<all_basic_types> "
        "  boost_container_types = "
        "boost::numeric::ublas::matrix<all_basic_types> | "
        "boost::multi_array<all_basic_types> \n"
        "  all_supported_types = all_basic_types | stl_container_types | "
        "boost_container_types"
        "  eigen_matrix_type = Eigen::Matrix<signed_arithmetic_type> | Eigen::VectorXX");
    return DataSpace(DataSpace::datascape_scalar);
}

template <typename Value>
inline DataSpace DataSpace::From(const std::vector<Value>& container) {
    return DataSpace(details::get_dim_vector<Value>(container));
}

template <typename ValueT, std::size_t N>
inline DataSpace DataSpace::From(const ValueT(&container)[N]) {
    return DataSpace(details::get_dim_vector(container));
}

template <std::size_t N, std::size_t Width>
inline DataSpace DataSpace::FromCharArrayStrings(const char(&)[N][Width]) {
    return DataSpace(N);
}

/// Currently only supports 1D std::array
template <typename Value, std::size_t N>
inline DataSpace DataSpace::From(const std::array<Value, N>& ) {
    return DataSpace(N);
}

#ifdef H5_USE_BOOST
template <typename Value, std::size_t Dims>
inline DataSpace
DataSpace::From(const boost::multi_array<Value, Dims>& container) {
    std::vector<size_t> dims(container.shape(), container.shape() + Dims);
    return DataSpace(dims);
}

template <typename Value>
inline DataSpace
DataSpace::From(const boost::numeric::ublas::matrix<Value>& mat) {
    std::vector<size_t> dims(2);
    dims[0] = mat.size1();
    dims[1] = mat.size2();
    return DataSpace(dims);
}
#endif

#ifdef H5_USE_EIGEN
template <typename Value, int M, int N>
inline DataSpace
DataSpace::From(const Eigen::Matrix<Value, M, N>&  mat ) {
    std::vector<size_t> dims{static_cast<size_t>(mat.rows()),
                             static_cast<size_t>(mat.cols())};
    return DataSpace(dims);
}

template <typename Value, int M, int N>
inline DataSpace
DataSpace::From(const std::vector<Eigen::Matrix<Value, M, N>>& vec) {
    using elem_t = Eigen::Matrix<Value, M, N>;
    std::vector<size_t> dims{
        std::accumulate(vec.begin(), vec.end(), size_t{0u},
                        [](size_t so_far, const elem_t& v) {
                            return so_far + static_cast<size_t>(v.rows());
                        }),
        static_cast<size_t>(vec[0].cols())};
    return DataSpace(dims);
}

#ifdef H5_USE_BOOST
template <typename Value, int M, int N, size_t Dims>
inline DataSpace DataSpace::
From(const boost::multi_array<Eigen::Matrix<Value, M, N>, Dims>& vec) {
    std::vector<size_t> dims(vec.shape(), vec.shape() + Dims);
    dims[Dims - 1] *= static_cast<size_t>(vec.origin()->rows() * vec.origin()->cols());
    return DataSpace(dims);
}
#endif

#endif

namespace details {

/// dimension checks @internal
inline bool checkDimensions(const DataSpace& mem_space, size_t input_dims) {
    size_t dataset_dims = mem_space.getNumberDimensions();
    if (input_dims == dataset_dims)
        return true;

    const std::vector<size_t>& dims = mem_space.getDimensions();
    for (auto i = dims.crbegin(); i != --dims.crend() && *i == 1; ++i)
        --dataset_dims;

    if (input_dims == dataset_dims)
        return true;

    dataset_dims = dims.size();
    for (auto i = dims.cbegin(); i != --dims.cend() && *i == 1; ++i)
        --dataset_dims;

    if (input_dims == dataset_dims)
        return true;

    // The final tests is for scalars
    return input_dims == 0 && dataset_dims == 1 && dims[dims.size() - 1] == 1;
}

} // namespace details
} // namespace HighFive

#endif // H5DATASPACE_MISC_HPP
