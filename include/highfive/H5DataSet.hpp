/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5DATASET_HPP
#define H5DATASET_HPP

#include <vector>

#include "H5Object.hpp"
#include "bits/H5Annotate_traits.hpp"
#include "bits/H5Slice_traits.hpp"

namespace HighFive {

template <typename Derivate>
class NodeTraits;
template <typename Derivate>
class SliceTraits;
class DataType;
class DataSpace;

class DataSet : public Object,
                public SliceTraits<DataSet>,
                public AnnotateTraits<DataSet> {
  public:
    size_t getStorageSize() const;

    ///
    /// \brief getDataType
    /// \return return the datatype associated with this dataset
    ///
    DataType getDataType() const;

    ///
    /// \brief getSpace
    /// \return return the dataspace associated with this dataset
    ///
    DataSpace getSpace() const;

    ///
    /// \brief getMemSpace
    /// \return same than getSpace for DataSet, compatibility with Selection
    /// class
    ///
    DataSpace getMemSpace() const;

    ///
    /// \brief getOffset
    /// \return returns DataSet address in file
    /// class
    ///
    size_t getOffset() const;

    /// \brief Change the size of the dataset
    ///
    /// This requires that the dataset was created with chunking, and you would
    /// generally want to have set a larger maxdims setting
    /// \param dims New size of the dataset
    void resize(const std::vector<size_t>& dims);


    /// \brief Get the dimensions of the whole DataSet.
    ///       This is a shorthand for getSpace().getDimensions()
    /// \return The shape of the current HighFive::DataSet
    ///
    inline std::vector<size_t> getDimensions() const {
        return getSpace().getDimensions();
    }

    /// \brief Get the total number of elements in the current dataset.
    ///       E.g. 2x2x2 matrix has size 8.
    ///       This is a shorthand for getSpace().getTotalCount()
    /// \return The shape of the current HighFive::DataSet
    ///
    inline size_t getElementCount() const {
        return getSpace().getElementCount();
    }

  private:
    DataSet();
    template <typename Derivate>
    friend class ::HighFive::NodeTraits;
};

}  // namespace HighFive

#include "bits/H5DataSet_misc.hpp"

#endif // H5DATASET_HPP
