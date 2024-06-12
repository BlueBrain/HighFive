/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <sstream>
#include <string>

#include "h5d_wrapper.hpp"
#include "h5s_wrapper.hpp"

#include "H5ReadWrite_misc.hpp"
#include "H5Converter_misc.hpp"
#include "squeeze.hpp"
#include "compute_total_size.hpp"
#include "assert_compatible_spaces.hpp"

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

namespace detail {
class HyperCube {
  public:
    HyperCube(size_t rank)
        : offset(rank)
        , count(rank) {}

    void cross(const std::array<size_t, 2>& range, size_t axis) {
        offset[axis] = range[0];
        count[axis] = range[1] - range[0];
    }

    RegularHyperSlab asSlab() {
        return RegularHyperSlab(offset, count);
    }

  private:
    std::vector<size_t> offset;
    std::vector<size_t> count;
};

inline void build_hyper_slab(HyperSlab& slab, size_t /* axis */, HyperCube& cube) {
    slab |= cube.asSlab();
}

template <class... Slices>
inline void build_hyper_slab(HyperSlab& slab,
                             size_t axis,
                             HyperCube& cube,
                             const std::array<size_t, 2>& slice,
                             const Slices&... higher_slices) {
    cube.cross(slice, axis);
    build_hyper_slab(slab, axis + 1, cube, higher_slices...);
}

template <class... Slices>
inline void build_hyper_slab(HyperSlab& slab,
                             size_t axis,
                             HyperCube& cube,
                             const std::vector<std::array<size_t, 2>>& slices,
                             const Slices&... higher_slices) {
    for (const auto& slice: slices) {
        build_hyper_slab(slab, axis, cube, slice, higher_slices...);
    }
}

template <class... Slices>
inline void build_hyper_slab(HyperSlab& slab,
                             size_t axis,
                             HyperCube& cube,
                             const std::vector<size_t>& ids,
                             const Slices&... higher_slices) {
    for (const auto& id: ids) {
        auto slice = std::array<size_t, 2>{id, id + 1};
        build_hyper_slab(slab, axis, cube, slice, higher_slices...);
    }
}

template <class... Slices>
inline void build_hyper_slab(HyperSlab& slab,
                             size_t axis,
                             HyperCube& cube,
                             size_t id,
                             const Slices&... higher_slices) {
    auto slice = std::array<size_t, 2>{id, id + 1};
    build_hyper_slab(slab, axis, cube, slice, higher_slices...);
}

inline void compute_squashed_shape(size_t /* axis */, std::vector<size_t>& /* shape */) {
    // assert(axis == shape.size());
}

template <class... Slices>
inline void compute_squashed_shape(size_t axis,
                                   std::vector<size_t>& shape,
                                   const std::array<size_t, 2>& slice,
                                   const Slices&... higher_slices);

template <class... Slices>
inline void compute_squashed_shape(size_t axis,
                                   std::vector<size_t>& shape,
                                   const std::vector<size_t>& points,
                                   const Slices&... higher_slices);

template <class... Slices>
inline void compute_squashed_shape(size_t axis,
                                   std::vector<size_t>& shape,
                                   size_t point,
                                   const Slices&... higher_slices);

template <class... Slices>
inline void compute_squashed_shape(size_t axis,
                                   std::vector<size_t>& shape,
                                   const std::vector<std::array<size_t, 2>>& slices,
                                   const Slices&... higher_slices);

template <class... Slices>
inline void compute_squashed_shape(size_t axis,
                                   std::vector<size_t>& shape,
                                   const std::array<size_t, 2>& slice,
                                   const Slices&... higher_slices) {
    shape[axis] = slice[1] - slice[0];
    compute_squashed_shape(axis + 1, shape, higher_slices...);
}

template <class... Slices>
inline void compute_squashed_shape(size_t axis,
                                   std::vector<size_t>& shape,
                                   const std::vector<size_t>& points,
                                   const Slices&... higher_slices) {
    shape[axis] = points.size();
    compute_squashed_shape(axis + 1, shape, higher_slices...);
}

template <class... Slices>
inline void compute_squashed_shape(size_t axis,
                                   std::vector<size_t>& shape,
                                   const std::vector<std::array<size_t, 2>>& slices,
                                   const Slices&... higher_slices) {
    shape[axis] = 0;
    for (const auto& slice: slices) {
        shape[axis] += slice[1] - slice[0];
    }
    compute_squashed_shape(axis + 1, shape, higher_slices...);
}

template <class... Slices>
inline void compute_squashed_shape(size_t axis,
                                   std::vector<size_t>& shape,
                                   size_t /* point */,
                                   const Slices&... higher_slices) {
    shape[axis] = 1;
    compute_squashed_shape(axis + 1, shape, higher_slices...);
}
}  // namespace detail

template <class... Slices>
inline ProductSet::ProductSet(const Slices&... slices) {
    auto rank = sizeof...(slices);
    detail::HyperCube cube(rank);
    detail::build_hyper_slab(slab, 0, cube, slices...);

    shape = std::vector<size_t>(rank, size_t(0));
    detail::compute_squashed_shape(0, shape, slices...);
}


template <typename Derivate>
inline Selection SliceTraits<Derivate>::select(const HyperSlab& hyperslab,
                                               const DataSpace& memspace) const {
    // Note: The current limitation are that memspace must describe a
    //       packed memspace.
    //
    //       The reason for this is that we're unable to unpack general
    //       hyperslabs when the memory is not contiguous, e.g.
    //       `std::vector<std::vector<double>>`.
    const auto& slice = static_cast<const Derivate&>(*this);
    auto filespace = hyperslab.apply(slice.getSpace());

    return detail::make_selection(memspace, filespace, details::get_dataset(slice));
}

template <typename Derivate>
inline Selection SliceTraits<Derivate>::select(const HyperSlab& hyper_slab) const {
    const auto& slice = static_cast<const Derivate&>(*this);
    auto filespace = slice.getSpace();
    filespace = hyper_slab.apply(filespace);

    auto n_elements = detail::h5s_get_select_npoints(filespace.getId());
    auto memspace = DataSpace(std::array<size_t, 1>{size_t(n_elements)});

    return detail::make_selection(memspace, filespace, details::get_dataset(slice));
}


template <typename Derivate>
inline Selection SliceTraits<Derivate>::select(const std::vector<size_t>& offset,
                                               const std::vector<size_t>& count,
                                               const std::vector<size_t>& stride,
                                               const std::vector<size_t>& block) const {
    auto slab = HyperSlab(RegularHyperSlab(offset, count, stride, block));
    auto memspace = DataSpace(count);
    return select(slab, memspace);
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

    return select(slab, DataSpace(memdims));
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

    detail::h5s_select_elements(space.getId(), H5S_SELECT_SET, num_elements, data);

    return detail::make_selection(DataSpace(num_elements), space, details::get_dataset(slice));
}

template <typename Derivate>
inline Selection SliceTraits<Derivate>::select(const ProductSet& product_set) const {
    return this->select(product_set.slab, DataSpace(product_set.shape));
}


template <typename Derivate>
template <typename T>
inline T SliceTraits<Derivate>::read(const DataTransferProps& xfer_props) const {
    T array;
    read(array, xfer_props);
    return array;
}


template <typename Derivate>
template <typename T>
inline void SliceTraits<Derivate>::read(T& array, const DataTransferProps& xfer_props) const {
    const auto& slice = static_cast<const Derivate&>(*this);
    const DataSpace& mem_space = slice.getMemSpace();

    auto file_datatype = slice.getDataType();

    const details::BufferInfo<T> buffer_info(
        file_datatype,
        [&slice]() -> std::string { return details::get_dataset(slice).getPath(); },
        details::BufferInfo<T>::Operation::read);

    if (!details::checkDimensions(mem_space, buffer_info.getMinRank(), buffer_info.getMaxRank())) {
        std::ostringstream ss;
        ss << "Impossible to read DataSet of dimensions " << mem_space.getNumberDimensions()
           << " into arrays of dimensions: " << buffer_info.getMinRank() << "(min) to "
           << buffer_info.getMaxRank() << "(max)";
        throw DataSpaceException(ss.str());
    }
    auto dims = mem_space.getDimensions();

    auto r = details::data_converter::get_reader<T>(dims, array, file_datatype);
    read_raw(r.getPointer(), buffer_info.data_type, xfer_props);
    // re-arrange results
    r.unserialize(array);

    auto t = buffer_info.data_type;
    auto c = t.getClass();
    if (c == DataTypeClass::VarLen || t.isVariableStr()) {
#if H5_VERSION_GE(1, 12, 0)
        // This one have been created in 1.12.0
        (void)
            detail::h5t_reclaim(t.getId(), mem_space.getId(), xfer_props.getId(), r.getPointer());
#else
        // This one is deprecated since 1.12.0
        (void) detail::h5d_vlen_reclaim(t.getId(),
                                        mem_space.getId(),
                                        xfer_props.getId(),
                                        r.getPointer());
#endif
    }
}


template <typename Derivate>
template <typename T>
inline void SliceTraits<Derivate>::read_raw(T* array,
                                            const DataType& mem_datatype,
                                            const DataTransferProps& xfer_props) const {
    static_assert(!std::is_const<T>::value,
                  "read() requires a non-const structure to read data into");

    const auto& slice = static_cast<const Derivate&>(*this);

    detail::h5d_read(details::get_dataset(slice).getId(),
                     mem_datatype.getId(),
                     details::get_memspace_id(slice),
                     slice.getSpace().getId(),
                     xfer_props.getId(),
                     static_cast<void*>(array));
}


template <typename Derivate>
template <typename T>
inline void SliceTraits<Derivate>::read_raw(T* array, const DataTransferProps& xfer_props) const {
    using element_type = typename details::inspector<T>::base_type;
    const DataType& mem_datatype = create_and_check_datatype<element_type>();

    read_raw(array, mem_datatype, xfer_props);
}


template <typename Derivate>
template <typename T>
inline void SliceTraits<Derivate>::write(const T& buffer, const DataTransferProps& xfer_props) {
    const auto& slice = static_cast<const Derivate&>(*this);
    const DataSpace& mem_space = slice.getMemSpace();
    auto dims = mem_space.getDimensions();

    auto file_datatype = slice.getDataType();

    const details::BufferInfo<T> buffer_info(
        file_datatype,
        [&slice]() -> std::string { return details::get_dataset(slice).getPath(); },
        details::BufferInfo<T>::Operation::write);

    if (!details::checkDimensions(mem_space, buffer_info.getMinRank(), buffer_info.getMaxRank())) {
        std::ostringstream ss;
        ss << "Impossible to write buffer with dimensions n = " << buffer_info.getRank(buffer)
           << "into dataset with dimensions " << details::format_vector(mem_space.getDimensions())
           << ".";
        throw DataSpaceException(ss.str());
    }
    auto w = details::data_converter::serialize<T>(buffer, dims, file_datatype);
    write_raw(w.getPointer(), buffer_info.data_type, xfer_props);
}


template <typename Derivate>
template <typename T>
inline void SliceTraits<Derivate>::write_raw(const T* buffer,
                                             const DataType& mem_datatype,
                                             const DataTransferProps& xfer_props) {
    const auto& slice = static_cast<const Derivate&>(*this);

    detail::h5d_write(details::get_dataset(slice).getId(),
                      mem_datatype.getId(),
                      details::get_memspace_id(slice),
                      slice.getSpace().getId(),
                      xfer_props.getId(),
                      static_cast<const void*>(buffer));
}


template <typename Derivate>
template <typename T>
inline void SliceTraits<Derivate>::write_raw(const T* buffer, const DataTransferProps& xfer_props) {
    using element_type = typename details::inspector<T>::base_type;
    const auto& mem_datatype = create_and_check_datatype<element_type>();

    write_raw(buffer, mem_datatype, xfer_props);
}

namespace detail {
inline const DataSet& getDataSet(const Selection& selection) {
    return selection.getDataset();
}

inline const DataSet& getDataSet(const DataSet& dataset) {
    return dataset;
}

}  // namespace detail

template <typename Derivate>
inline Selection SliceTraits<Derivate>::squeezeMemSpace(const std::vector<size_t>& axes) const {
    auto slice = static_cast<const Derivate&>(*this);
    auto mem_dims = slice.getMemSpace().getDimensions();
    auto squeezed_dims = detail::squeeze(mem_dims, axes);

    return detail::make_selection(DataSpace(squeezed_dims),
                                  slice.getSpace(),
                                  detail::getDataSet(slice));
}

template <typename Derivate>
inline Selection SliceTraits<Derivate>::reshapeMemSpace(const std::vector<size_t>& new_dims) const {
    auto slice = static_cast<const Derivate&>(*this);

    detail::assert_compatible_spaces(slice.getMemSpace(), new_dims);
    return detail::make_selection(DataSpace(new_dims), slice.getSpace(), detail::getDataSet(slice));
}

}  // namespace HighFive
