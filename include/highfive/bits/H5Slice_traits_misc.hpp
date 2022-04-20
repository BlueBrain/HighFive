/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5SLICE_TRAITS_MISC_HPP
#define H5SLICE_TRAITS_MISC_HPP

#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <sstream>
#include <string>

#ifdef H5_USE_BOOST
// starting Boost 1.64, serialization header must come before ublas
#include <boost/multi_array.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/serialization/vector.hpp>
#endif

#include <H5Dpublic.h>
#include <H5Ppublic.h>

#include "H5ReadWrite_misc.hpp"
#include "H5Converter_misc.hpp"

namespace HighFive {

namespace details {

// map the correct reference to the dataset depending of the layout
// dataset -> itself
// subselection -> parent dataset
inline const DataSet& get_dataset(const Selection& sel) {
    return sel.getDataset();
}

inline const DataSet& get_dataset(const DataSet& ds) {
    return ds;
}

// map the correct memspace identifier depending of the layout
// dataset -> entire memspace
// selection -> resolve space id
inline hid_t get_memspace_id(const Selection& ptr) {
    return ptr.getMemSpace().getId();
}

inline hid_t get_memspace_id(const DataSet&) {
    return H5S_ALL;
}
}  // namespace details

inline ElementSet::ElementSet(std::initializer_list<std::size_t> list)
    : _ids(list) {}

inline ElementSet::ElementSet(std::initializer_list<std::vector<std::size_t>> list)
    : ElementSet(std::vector<std::vector<std::size_t>>(list)) {}

inline ElementSet::ElementSet(const std::vector<std::size_t>& element_ids)
    : _ids(element_ids) {}

inline ElementSet::ElementSet(const std::vector<std::vector<std::size_t>>& element_ids) {
    for (const auto& vec: element_ids) {
        std::copy(vec.begin(), vec.end(), std::back_inserter(_ids));
    }
}

template <typename Derivate>
inline Selection SliceTraits<Derivate>::select_impl(const HyperSlab& hyperslab,
                                                    const DataSpace& memspace) const {
    // Note: The current limitation are that memspace must describe a
    //       packed memspace.
    //
    //       The reason for this is that we're unable to unpack general
    //       hyperslabs when the memory is not contiguous, e.g.
    //       `std::vector<std::vector<double>>`.
    const auto& slice = static_cast<const Derivate&>(*this);
    auto filespace = hyperslab.apply(slice.getSpace());

    return Selection(memspace, filespace, details::get_dataset(slice));
}

template <typename Derivate>
inline Selection SliceTraits<Derivate>::select(const HyperSlab& hyper_slab) const {
    const auto& slice = static_cast<const Derivate&>(*this);
    auto filespace = slice.getSpace();
    filespace = hyper_slab.apply(filespace);

    auto n_elements = H5Sget_select_npoints(filespace.getId());
    auto memspace = DataSpace(std::array<size_t, 1>{size_t(n_elements)});

    return Selection(memspace, filespace, details::get_dataset(slice));
}


template <typename Derivate>
inline Selection SliceTraits<Derivate>::select(const std::vector<size_t>& offset,
                                               const std::vector<size_t>& count,
                                               const std::vector<size_t>& stride) const {
    auto slab = HyperSlab(RegularHyperSlab(offset, count, stride));
    auto memspace = DataSpace(count);
    return select_impl(slab, memspace);
}

template <typename Derivate>
inline Selection SliceTraits<Derivate>::select(const std::vector<size_t>& columns) const {
    const auto& slice = static_cast<const Derivate&>(*this);
    const DataSpace& space = slice.getSpace();
    std::vector<size_t> dims = space.getDimensions();

    std::vector<size_t> counts = dims;
    counts.back() = 1;

    std::vector<size_t> offsets(dims.size(), 0);

    HyperSlab slab;
    for (const auto& column: columns) {
        offsets.back() = column;
        slab |= RegularHyperSlab(offsets, counts);
    }

    std::vector<size_t> memdims = dims;
    memdims.back() = columns.size();

    return select_impl(slab, DataSpace(memdims));
}

template <typename Derivate>
inline Selection SliceTraits<Derivate>::select(const ElementSet& elements) const {
    const auto& slice = static_cast<const Derivate&>(*this);
    const hsize_t* data = nullptr;
    const DataSpace space = slice.getSpace().clone();
    const std::size_t length = elements._ids.size();
    if (length % space.getNumberDimensions() != 0) {
        throw DataSpaceException(
            "Number of coordinates in elements picking "
            "should be a multiple of the dimensions.");
    }
    const std::size_t num_elements = length / space.getNumberDimensions();
    std::vector<hsize_t> raw_elements;

    // optimised at compile time
    // switch for data conversion on 32bits platforms
    if (std::is_same<std::size_t, hsize_t>::value) {
        // `if constexpr` can't be used, thus a reinterpret_cast is needed.
        data = reinterpret_cast<const hsize_t*>(&(elements._ids[0]));
    } else {
        raw_elements.resize(length);
        std::copy(elements._ids.begin(), elements._ids.end(), raw_elements.begin());
        data = raw_elements.data();
    }

    if (H5Sselect_elements(space.getId(), H5S_SELECT_SET, num_elements, data) < 0) {
        HDF5ErrMapper::ToException<DataSpaceException>("Unable to select elements");
    }

    return Selection(DataSpace(num_elements), space, details::get_dataset(slice));
}


template <typename Derivate>
template <typename T>
inline void SliceTraits<Derivate>::read(T& array) const {
    const auto& slice = static_cast<const Derivate&>(*this);
    const DataSpace& mem_space = slice.getMemSpace();
    const details::BufferInfo<T> buffer_info(slice.getDataType(), [slice]() -> std::string {
        return details::get_dataset(slice).getPath();
    });

    if (!details::checkDimensions(mem_space, buffer_info.n_dimensions)) {
        std::ostringstream ss;
        ss << "Impossible to read DataSet of dimensions " << mem_space.getNumberDimensions()
           << " into arrays of dimensions " << buffer_info.n_dimensions;
        throw DataSpaceException(ss.str());
    }
    details::data_converter<T> converter(mem_space);
    read(converter.transform_read(array), buffer_info.data_type);
    // re-arrange results
    converter.process_result(array);
}

template <typename Derivate>
template <typename T>
inline void SliceTraits<Derivate>::read(T* array, const DataType& dtype) const {
    static_assert(!std::is_const<T>::value,
                  "read() requires a non-const structure to read data into");
    const auto& slice = static_cast<const Derivate&>(*this);
    using element_type = typename details::inspector<T>::base_type;

    // Auto-detect mem datatype if not provided
    const DataType& mem_datatype = dtype.empty() ? create_and_check_datatype<element_type>()
                                                 : dtype;

    if (H5Dread(details::get_dataset(slice).getId(),
                mem_datatype.getId(),
                details::get_memspace_id(slice),
                slice.getSpace().getId(),
                H5P_DEFAULT,
                static_cast<void*>(array)) < 0) {
        HDF5ErrMapper::ToException<DataSetException>("Error during HDF5 Read: ");
    }
}


template <typename Derivate>
template <typename T>
inline void SliceTraits<Derivate>::write(const T& buffer) {
    const auto& slice = static_cast<const Derivate&>(*this);
    const DataSpace& mem_space = slice.getMemSpace();
    const details::BufferInfo<T> buffer_info(slice.getDataType(), [slice]() -> std::string {
        return details::get_dataset(slice).getPath();
    });

    if (!details::checkDimensions(mem_space, buffer_info.n_dimensions)) {
        std::ostringstream ss;
        ss << "Impossible to write buffer of dimensions " << buffer_info.n_dimensions
           << " into dataset of dimensions " << mem_space.getNumberDimensions();
        throw DataSpaceException(ss.str());
    }
    details::data_converter<T> converter(mem_space);
    write_raw(converter.transform_write(buffer), buffer_info.data_type);
}


template <typename Derivate>
template <typename T>
inline void SliceTraits<Derivate>::write_raw(const T* buffer, const DataType& dtype) {
    using element_type = typename details::inspector<T>::base_type;
    const auto& slice = static_cast<const Derivate&>(*this);
    const auto& mem_datatype = dtype.empty() ? create_and_check_datatype<element_type>() : dtype;

    if (H5Dwrite(details::get_dataset(slice).getId(),
                 mem_datatype.getId(),
                 details::get_memspace_id(slice),
                 slice.getSpace().getId(),
                 H5P_DEFAULT,
                 static_cast<const void*>(buffer)) < 0) {
        HDF5ErrMapper::ToException<DataSetException>("Error during HDF5 Write: ");
    }
}


}  // namespace HighFive

#endif  // H5SLICE_TRAITS_MISC_HPP
