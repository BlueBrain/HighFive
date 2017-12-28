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
    void setTranspose(const bool doTranspose);
    size_t getStorageSize() const;

    ///
    /// \brief getDataType
    /// \return return the datatype associated with this dataset
    ///
    DataType getDataType() const;

    std::vector<size_t> getDataDimensions() const;

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

    bool isTransposed() const;

  private:
    DataSet();

    bool doTranspose;
    template <typename Derivate>
    friend class ::HighFive::NodeTraits;
};
}

#include "bits/H5DataSet_misc.hpp"

#endif // H5DATASET_HPP
