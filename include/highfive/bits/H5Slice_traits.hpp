/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5SLICE_TRAITS_HPP
#define H5SLICE_TRAITS_HPP

#include <cstdlib>
#include <vector>

#include "H5_definitions.hpp"
#include "H5Utils.hpp"

namespace HighFive {

class ElementSet {
  public:
    ///
    /// \brief Create a list of points of N-dimension for selection.
    ///
    /// \param list List of continuous coordinates (eg.: in 2 dimensions space `ElementSet{1, 2, 3 ,4}` create points
    /// `(1, 2)` and `(3, 4)`).
    explicit ElementSet(std::initializer_list<std::size_t> list);
    ///
    /// \brief Create a list of points of N-dimension for selection.
    ///
    /// \param list List of N-dim points.
    explicit ElementSet(std::initializer_list<std::vector<std::size_t>> list);
    ///
    /// \brief Create a list of points of N-dimension for selection.
    ///
    /// \param element_ids List of continuous coordinates (eg.: in 2 dimensions space `ElementSet{1, 2, 3 ,4}` create points
    /// `(1, 2)` and `(3, 4)`).
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

class HyperSlab {
  public:
    struct Select {
        Select() = default;

        Select(std::vector<size_t> offset_, std::vector<size_t> count_, std::vector<size_t> stride_)
            : offset(offset_), count(count_), stride(stride_)
        {}

        std::vector<size_t> offset;
        std::vector<size_t> count;
        std::vector<size_t> stride;
    };

    HyperSlab() {
        selects.emplace_back(Select{}, Op::None);
    };

    HyperSlab(Select sel) {
        selects.emplace_back(sel, Op::Set);
    }

    HyperSlab operator|(const Select& sel) const {
        auto ret = *this;
        ret.selects.emplace_back(sel, Op::Or);
        return ret;
    }

    HyperSlab& operator|=(const Select& sel) {
        selects.emplace_back(sel, Op::Or);
        return *this;
    }

    HyperSlab operator&(const Select& sel) const {
        auto ret = *this;
        ret.selects.emplace_back(sel, Op::And);
        return ret;
    }

    HyperSlab& operator&=(const Select& sel) {
        selects.emplace_back(sel, Op::And);
        return *this;
    }

    HyperSlab operator^(const Select& sel) const {
        auto ret = *this;
        ret.selects.emplace_back(sel, Op::Xor);
        return ret;
    }

    HyperSlab& operator^=(const Select& sel) {
        selects.emplace_back(sel, Op::Xor);
        return *this;
    }

    void apply(const DataSpace& space) {
        for (const auto& sel: selects) {
            if (sel.op == Op::None) {
                H5Sselect_none(space.getId());
            } else {
                std::vector<hsize_t> offset_local(sel.offset.size());
                std::vector<hsize_t> count_local(sel.count.size());
                std::vector<hsize_t> stride_local(sel.stride.size());
                std::copy(sel.offset.begin(), sel.offset.end(), offset_local.begin());
                std::copy(sel.count.begin(), sel.count.end(), count_local.begin());
                std::copy(sel.stride.begin(), sel.stride.end(), stride_local.begin());
                if (H5Sselect_hyperslab(space.getId(), convert(sel.op), offset_local.data(), stride_local.empty() ? NULL : stride_local.data(), count_local.empty() ? NULL : count_local.data(), NULL) < 0) {
                    HDF5ErrMapper::ToException<DataSpaceException>("Unable to select hyperslab");
                }
            }
        }
    }

  private:
    enum Op {
        Set,
        Or,
        And,
        Xor,
        NotB,
        NotA,
        None,
    };

    H5S_seloper_t convert(Op op) {
        switch(op) {
          case None:
          case Set:
            return H5S_SELECT_SET;
          case Or:
            return H5S_SELECT_OR;
          case And:
            return H5S_SELECT_AND;
          case Xor:
            return H5S_SELECT_XOR;
          case NotB:
            return H5S_SELECT_NOTB;
          case NotA:
            return H5S_SELECT_NOTA;
        }
        return H5S_SELECT_SET;
    }

    struct Select_ {
        Select_(Select sel, Op op_)
            : offset(sel.offset), count(sel.count), stride(sel.stride), op(op_) 
        {}
        std::vector<size_t> offset;
        std::vector<size_t> count;
        std::vector<size_t> stride;
        Op op;
    };

    std::vector<Select_> selects;
};


template <typename Derivate>
class SliceTraits {
  public:
    ///
    /// \brief Select a region in the current Slice/Dataset of \p count points at
    /// \p offset separated by \p stride. If strides are not provided they will
    /// default to 1 in all dimensions.
    ///
    /// vector offset and count have to be from the same dimension
    ///
    Selection select(const std::vector<size_t>& offset,
                     const std::vector<size_t>& count,
                     const std::vector<size_t>& stride = std::vector<size_t>())
        const;

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
    /// Read the entire dataset into a buffer
    /// An exception is raised is if the numbers of dimension of the buffer and
    /// of the dataset are different.
    ///
    /// The array type can be a N-pointer or a N-vector. For plain pointers
    /// not dimensionality checking will be performed, it is the user's
    /// responsibility to ensure that the right amount of space has been
    /// allocated.
    template <typename T>
    void read(T& array) const;

    ///
    /// Read the entire dataset into a raw buffer
    ///
    /// No dimensionality checks will be performed, it is the user's
    /// responsibility to ensure that the right amount of space has been
    /// allocated.
    /// \param array: A buffer containing enough space for the data
    /// \param dtype: The type of the data, in case it cannot be automatically guessed
    template <typename T>
    void read(T* array, const DataType& dtype = DataType()) const;

    ///
    /// Write the integrality N-dimension buffer to this dataset
    /// An exception is raised is if the numbers of dimension of the buffer and
    /// of the dataset are different
    ///
    /// The array type can be a N-pointer or a N-vector ( e.g int** integer two
    /// dimensional array )
    template <typename T>
    void write(const T& buffer);

    ///
    /// Write from a raw buffer into this dataset
    ///
    /// No dimensionality checks will be performed, it is the user's
    /// responsibility to ensure that the buffer holds the right amount of
    /// elements. For n-dimensional matrices the buffer layout follows H5
    /// default conventions.
    /// \param buffer: A buffer containing the data to be written
    /// \param dtype: The type of the data, in case it cannot be automatically guessed
    template <typename T>
    void write_raw(const T* buffer, const DataType& dtype = DataType());

};

}  // namespace HighFive

#endif // H5SLICE_TRAITS_HPP
