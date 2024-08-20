/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <cstdlib>
#include <vector>

#include "H5_definitions.hpp"
#include "H5Utils.hpp"
#include "convert_size_vector.hpp"

#include "../H5PropertyList.hpp"
#include "h5s_wrapper.hpp"

namespace HighFive {

class ElementSet {
  public:
    ///
    /// \brief Create a list of points of N-dimension for selection.
    ///
    /// \param list List of continuous coordinates (e.g.: in 2 dimensions space
    /// `ElementSet{1, 2, 3 ,4}` creates points `(1, 2)` and `(3, 4)`).
    explicit ElementSet(std::initializer_list<std::size_t> list);
    ///
    /// \brief Create a list of points of N-dimension for selection.
    ///
    /// \param list List of N-dim points.
    explicit ElementSet(std::initializer_list<std::vector<std::size_t>> list);
    ///
    /// \brief Create a list of points of N-dimension for selection.
    ///
    /// \param element_ids List of continuous coordinates (e.g.: in 2 dimensions space
    /// `ElementSet{1, 2, 3 ,4}` creates points `(1, 2)` and `(3, 4)`).
    explicit ElementSet(const std::vector<std::size_t>& element_ids);
    ///
    /// \brief Create a list of points of N-dimension for selection.
    ///
    /// \param element_ids List of N-dim points.
    explicit ElementSet(const std::vector<std::vector<std::size_t>>& element_ids);

  private:
    std::vector<std::size_t> _ids;

    template <typename Derivate>
    friend class SliceTraits;
};

inline std::vector<hsize_t> toHDF5SizeVector(const std::vector<size_t>& from) {
    return detail::convertSizeVector<hsize_t>(from);
}

inline std::vector<size_t> toSTLSizeVector(const std::vector<hsize_t>& from) {
    return detail::convertSizeVector<size_t>(from);
}

struct RegularHyperSlab {
    RegularHyperSlab() = default;

    RegularHyperSlab(const std::vector<size_t>& offset_,
                     const std::vector<size_t>& count_ = {},
                     const std::vector<size_t>& stride_ = {},
                     const std::vector<size_t>& block_ = {})
        : offset(toHDF5SizeVector(offset_))
        , count(toHDF5SizeVector(count_))
        , stride(toHDF5SizeVector(stride_))
        , block(toHDF5SizeVector(block_)) {}

    static RegularHyperSlab fromHDF5Sizes(std::vector<hsize_t> offset_,
                                          std::vector<hsize_t> count_ = {},
                                          std::vector<hsize_t> stride_ = {},
                                          std::vector<hsize_t> block_ = {}) {
        RegularHyperSlab slab;
        slab.offset = std::move(offset_);
        slab.count = std::move(count_);
        slab.stride = std::move(stride_);
        slab.block = std::move(block_);

        return slab;
    }

    size_t rank() const {
        return std::max(std::max(offset.size(), count.size()),
                        std::max(stride.size(), block.size()));
    }

    /// Dimensions when all gaps are removed.
    std::vector<size_t> packedDims() const {
        auto n_dims = rank();
        auto dims = std::vector<size_t>(n_dims, 0);

        for (size_t i = 0; i < n_dims; ++i) {
            dims[i] = count[i] * (block.empty() ? 1 : block[i]);
        }

        return dims;
    }

    std::vector<hsize_t> offset;
    std::vector<hsize_t> count;
    std::vector<hsize_t> stride;
    std::vector<hsize_t> block;
};

class HyperSlab {
  public:
    HyperSlab() {
        selects.emplace_back(RegularHyperSlab{}, Op::None);
    };

    explicit HyperSlab(const RegularHyperSlab& sel) {
        selects.emplace_back(sel, Op::Set);
    }

    HyperSlab operator|(const RegularHyperSlab& sel) const {
        auto ret = *this;
        ret |= sel;
        return ret;
    }

    HyperSlab& operator|=(const RegularHyperSlab& sel) {
        selects.emplace_back(sel, Op::Or);
        return *this;
    }

    HyperSlab operator&(const RegularHyperSlab& sel) const {
        auto ret = *this;
        ret &= sel;
        return ret;
    }

    HyperSlab& operator&=(const RegularHyperSlab& sel) {
        selects.emplace_back(sel, Op::And);
        return *this;
    }

    HyperSlab operator^(const RegularHyperSlab& sel) const {
        auto ret = *this;
        ret ^= sel;
        return ret;
    }

    HyperSlab& operator^=(const RegularHyperSlab& sel) {
        selects.emplace_back(sel, Op::Xor);
        return *this;
    }

    HyperSlab& notA(const RegularHyperSlab& sel) {
        selects.emplace_back(sel, Op::NotA);
        return *this;
    }

    HyperSlab& notB(const RegularHyperSlab& sel) {
        selects.emplace_back(sel, Op::NotB);
        return *this;
    }

    DataSpace apply(const DataSpace& space_) const {
        return apply_impl(space_);
    }

  private:
    enum class Op {
        Noop,
        Set,
        Or,
        And,
        Xor,
        NotB,
        NotA,
        Append,
        Prepend,
        Invalid,
        None,
    };

    H5S_seloper_t convert(Op op) const {
        switch (op) {
        case Op::Noop:
            return H5S_SELECT_NOOP;
        case Op::Set:
            return H5S_SELECT_SET;
        case Op::Or:
            return H5S_SELECT_OR;
        case Op::And:
            return H5S_SELECT_AND;
        case Op::Xor:
            return H5S_SELECT_XOR;
        case Op::NotB:
            return H5S_SELECT_NOTB;
        case Op::NotA:
            return H5S_SELECT_NOTA;
        case Op::Append:
            return H5S_SELECT_APPEND;
        case Op::Prepend:
            return H5S_SELECT_PREPEND;
        case Op::Invalid:
            return H5S_SELECT_INVALID;
        default:
            throw DataSpaceException("Invalid HyperSlab operation.");
        }
    }

    struct Select_: public RegularHyperSlab {
        Select_(const RegularHyperSlab& sel, Op op_)
            : RegularHyperSlab(sel)
            , op(op_) {}

        Op op;
    };

    std::vector<Select_> selects;

  protected:
    DataSpace select_none(const DataSpace& outer_space) const {
        auto space = outer_space.clone();
        detail::h5s_select_none(space.getId());
        return space;
    }

    void select_hyperslab(DataSpace& space, const Select_& sel) const {
        detail::h5s_select_hyperslab(space.getId(),
                                     convert(sel.op),
                                     sel.offset.empty() ? nullptr : sel.offset.data(),
                                     sel.stride.empty() ? nullptr : sel.stride.data(),
                                     sel.count.empty() ? nullptr : sel.count.data(),
                                     sel.block.empty() ? nullptr : sel.block.data());
    }

#if H5_VERSION_GE(1, 10, 6)
    /// The length of a stream of `Op::Or` starting at `begin`.
    size_t detect_streak(Select_ const* begin, Select_ const* end, Op op) const {
        assert(op == Op::Or);
        auto it = std::find_if(begin, end, [op](const Select_& sel) { return sel.op != op; });
        return static_cast<size_t>(it - begin);
    }

    DataSpace combine_selections(const DataSpace& left_space,
                                 Op op,
                                 const DataSpace& right_space) const {
        assert(op == Op::Or);

        auto left_type = detail::h5s_get_select_type(left_space.getId());
        auto right_type = detail::h5s_get_select_type(right_space.getId());

        // Since HDF5 doesn't allow `combine_selections` with a None
        // selection, we need to avoid the issue:
        if (left_type == H5S_SEL_NONE) {
            return right_space;
        } else if (right_type == H5S_SEL_NONE) {
            return left_space;
        } else if (left_type == H5S_SEL_ALL) {
            return left_space;
        } else if (right_type == H5S_SEL_ALL) {
            return right_space;
        } else {
            return detail::make_data_space(
                detail::h5s_combine_select(left_space.getId(), convert(op), right_space.getId()));
        }
    }

    /// Reduce a sequence of `Op::Or` efficiently.
    ///
    /// The issue is that `H5Sselect_hyperslab` runs in time that linear of the
    /// number of block in the existing selection. Therefore, a loop that adds
    /// slab-by-slab has quadratic runtime in the number of slabs.
    ///
    /// Fortunately, `H5Scombine_select` doesn't suffer from the same problem.
    /// However, it's only available in 1.10.6 and newer.
    ///
    /// The solution is to use divide-and-conquer to reduce (long) streaks of
    /// `Op::Or` in what seems to be log-linear time.
    DataSpace reduce_streak(const DataSpace& outer_space,
                            Select_ const* begin,
                            Select_ const* end,
                            Op op) const {
        assert(op == Op::Or);

        if (begin == end) {
            throw std::runtime_error("Broken logic in 'DataSpace::reduce_streak'.");
        }

        std::ptrdiff_t distance = end - begin;
        if (distance == 1) {
            auto space = select_none(outer_space);
            select_hyperslab(space, *begin);
            return space;
        }

        Select_ const* mid = begin + distance / 2;
        auto right_space = reduce_streak(outer_space, begin, mid, op);
        auto left_space = reduce_streak(outer_space, mid, end, op);

        return combine_selections(left_space, op, right_space);
    }

    DataSpace apply_impl(const DataSpace& space_) const {
        auto space = space_.clone();
        auto n_selects = selects.size();
        for (size_t i = 0; i < n_selects; ++i) {
            auto begin = selects.data() + i;
            auto end = selects.data() + n_selects;

            auto n_ors = detect_streak(begin, end, Op::Or);

            if (n_ors > 1) {
                auto right_space = reduce_streak(space_, begin, begin + n_ors, Op::Or);
                space = combine_selections(space, Op::Or, right_space);
                i += n_ors - 1;
            } else if (selects[i].op == Op::None) {
                detail::h5s_select_none(space.getId());
            } else {
                select_hyperslab(space, selects[i]);
            }
        }
        return space;
    }
#else
    DataSpace apply_impl(const DataSpace& space_) const {
        auto space = space_.clone();
        for (const auto& sel: selects) {
            if (sel.op == Op::None) {
                detail::h5s_select_none(space.getId());
            } else {
                select_hyperslab(space, sel);
            }
        }
        return space;
    }
#endif
};

///
/// \brief Selects the Cartesian product of slices.
///
/// Given a one-dimensional dataset one might want to select the union of
/// multiple, non-overlapping slices. For example,
///
///     using Slice = std::array<size_t, 2>;
///     using Slices = std::vector<Slice>;
///     auto slices = Slices{{0, 2}, {4, 10}};
///     dset.select(ProductSet(slices);
///
/// to select elements `0`, `1` and `4`, ..., `9` (inclusive).
///
/// For a two-dimensional array one which to select the row specified above,
/// but only columns `2`, `3` and `4`:
///
///    dset.select(ProductSet(slices, Slice{2, 5}));
///    // Analogues with the roles of columns and rows reversed:
///    dset.select(ProductSet(Slice{2, 5}, slices));
///
/// One can generalize once more and allow the unions of slices in both x- and
/// y-dimension:
///
///     auto yslices = Slices{{1, 5}, {7, 8}};
///     auto xslices = Slices{{0, 3}, {6, 8}};
///     dset.select(ProductSet(yslices, xslices));
///
/// which selects the following from a 11x8 dataset:
///
///     . . . . . . . .
///     x x x . . . x x
///     x x x . . . x x
///     x x x . . . x x
///     x x x . . . x x
///     . . . . . . . .
///     . . . . . . . .
///     x x x . . . x x
///     . . . . . . . .
///     . . . . . . . .
///     . . . . . . . .
///
/// Final twist, the selection along and axis may be discrete indices, from
/// which a vector of, possibly single-element, slices can be constructed. The
/// corresponding types are `std::vector<size_t>` and `size_t` for multiple or
/// just a single values. Note, looping over rows or columns one-by-one can be
/// a very serious performance problem. In particular,
///
///     // Avoid:
///     for(auto i : indices) {
///         dset.select(i).read<double>();
///     }
///
///     // Use:
///     std::vector<size_t> tmp(indices.begin(), indices.end());
///     dset.select(tmp).read<std::vector<double>>();
///
/// obvious omit the copy if `indices` already has the correct type.
///
/// The solution works analogous in higher dimensions. A selection `sk` along
/// axis `k` can be interpreted as a subset `S_k` of the natural numbers. The
/// index `i` is in `S_k` if it's selected by `sk`.  The `ProductSet` of `s0`,
/// ..., `sN` selects the Cartesian product `S_0 x ... x S_N`.
///
/// Note that the selections along each axis must be sorted and non-overlapping.
///
/// \since 3.0
class ProductSet {
  public:
    template <class... Slices>
    explicit ProductSet(const Slices&... slices);

  private:
    HyperSlab slab;
    std::vector<size_t> shape;

    template <typename Derivate>
    friend class SliceTraits;
};


template <typename Derivate>
class SliceTraits {
  public:
    ///
    /// \brief Select an \p hyperslab in the current Slice/Dataset.
    ///
    /// HyperSlabs can be either regular or irregular. Irregular hyperslabs are typically generated
    /// by taking the union of regular hyperslabs. An irregular hyperslab, in general, does not fit
    /// nicely into a multi-dimensional array, but only a subset of such an array.
    ///
    /// Therefore, the only memspaces supported for general hyperslabs are one-dimensional arrays.
    Selection select(const HyperSlab& hyperslab) const;

    ///
    /// \brief Select an \p hyperslab in the current Slice/Dataset.
    ///
    /// If the selection can be read into a simple, multi-dimensional dataspace,
    /// then this overload enable specifying the shape of the memory dataspace
    /// with `memspace`. Note, that simple implies no offsets, strides or
    /// number of blocks, just the size of the block in each dimension.
    Selection select(const HyperSlab& hyperslab, const DataSpace& memspace) const;

    ///
    /// \brief Select a region in the current Slice/Dataset of \p count points at
    /// \p offset separated by \p stride. If strides are not provided they will
    /// default to 1 in all dimensions.
    ///
    /// vector offset and count have to be from the same dimension
    ///
    Selection select(const std::vector<size_t>& offset,
                     const std::vector<size_t>& count,
                     const std::vector<size_t>& stride = {},
                     const std::vector<size_t>& block = {}) const;

    ///
    /// \brief Select a set of columns in the last dimension of this dataset.
    ///
    /// The column indices must be smaller than the dimension size.
    ///
    Selection select(const std::vector<size_t>& columns) const;

    ///
    /// \brief Select a region in the current Slice/Dataset out of a list of elements.
    ///
    Selection select(const ElementSet& elements) const;

    ///
    /// \brief Select a region consisting of a product of slices.
    ///
    /// \since 3.0
    ///
    Selection select(const ProductSet& product_set) const;

    template <typename T>
    T read(const DataTransferProps& xfer_props = DataTransferProps()) const;

    ///
    /// Read the entire dataset into a buffer
    ///
    /// An exception is raised is if the numbers of dimension of the buffer and
    /// of the dataset are different.
    ///
    /// The array type can be a N-pointer or a N-vector. For plain pointers
    /// not dimensionality checking will be performed, it is the user's
    /// responsibility to ensure that the right amount of space has been
    /// allocated.
    template <typename T>
    void read(T& array, const DataTransferProps& xfer_props = DataTransferProps()) const;

    ///
    /// Read the entire dataset into a raw buffer
    ///
    /// No dimensionality checks will be performed, it is the user's
    /// responsibility to ensure that the right amount of space has been
    /// allocated.
    /// \param array: A buffer containing enough space for the data
    /// \param dtype: The type of the data, in case it cannot be automatically guessed
    /// \param xfer_props: Data Transfer properties
    template <typename T>
    void read_raw(T* array,
                  const DataType& dtype,
                  const DataTransferProps& xfer_props = DataTransferProps()) const;

    ///
    /// Read the entire dataset into a raw buffer
    ///
    /// Same as `read(T*, const DataType&, const DataTransferProps&)`. However,
    /// this overload deduces the HDF5 datatype of the element of `array` from
    /// `T`. Note, that the file datatype is already fixed.
    ///
    /// \param array: A buffer containing enough space for the data
    /// \param xfer_props: Data Transfer properties
    template <typename T>
    void read_raw(T* array, const DataTransferProps& xfer_props = DataTransferProps()) const;


    ///
    /// Write the integrality N-dimension buffer to this dataset
    /// An exception is raised is if the numbers of dimension of the buffer and
    /// of the dataset are different
    ///
    /// The array type can be a N-pointer or a N-vector ( e.g int** integer two
    /// dimensional array )
    template <typename T>
    void write(const T& buffer, const DataTransferProps& xfer_props = DataTransferProps());

    ///
    /// Write from a raw pointer into this dataset.
    ///
    /// No dimensionality checks will be performed, it is the user's
    /// responsibility to ensure that the buffer holds the right amount of
    /// elements. For n-dimensional matrices the buffer layout follows H5
    /// default conventions.
    ///
    /// Note, this is the shallowest wrapper around `H5Dwrite` and should
    /// be used if full control is needed. Generally prefer `write`.
    ///
    /// \param buffer: A buffer containing the data to be written
    /// \param dtype: The datatype of `buffer`, i.e. the memory data type.
    /// \param xfer_props: The HDF5 data transfer properties, e.g. collective MPI-IO.
    template <typename T>
    void write_raw(const T* buffer,
                   const DataType& mem_datatype,
                   const DataTransferProps& xfer_props = DataTransferProps());

    ///
    /// Write from a raw pointer into this dataset.
    ///
    /// Same as `write_raw(const T*, const DataTransferProps&)`. However, this
    /// overload attempts to guess the data type of `buffer`, i.e. the memory
    /// datatype. Note that the file datatype is already fixed.
    ///
    template <typename T>
    void write_raw(const T* buffer, const DataTransferProps& xfer_props = DataTransferProps());

    ///
    /// \brief Return a `Selection` with `axes` squeezed from the memspace.
    ///
    /// Returns a selection in which the memspace has been modified
    /// to not include the axes listed in `axes`.
    ///
    /// Throws if any axis to be squeezes has a dimension other than `1`.
    ///
    /// \since 3.0
    Selection squeezeMemSpace(const std::vector<size_t>& axes) const;

    ///
    /// \brief Return a `Selection` with a simple memspace with `dims`.
    ///
    /// Returns a selection in which the memspace has been modified
    /// to be a simple dataspace with dimensions `dims`.
    ///
    /// Throws if the number of elements changes.
    ///
    /// \since 3.0
    Selection reshapeMemSpace(const std::vector<size_t>& dims) const;
};

}  // namespace HighFive
