/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include "H5DataSet.hpp"
#include "H5DataSpace.hpp"
#include "bits/H5Slice_traits.hpp"
#include "bits/H5Friends.hpp"

namespace HighFive {

namespace detail {
Selection make_selection(const DataSpace&, const DataSpace&, const DataSet&);
}

///
/// \brief Selection: represent a view on a slice/part of a dataset
///
/// A Selection is valid only if its parent dataset is valid
///
class Selection: public SliceTraits<Selection> {
  public:
    ///
    /// \brief getSpace
    /// \return Dataspace associated with this selection
    ///
    DataSpace getSpace() const;

    ///
    /// \brief getMemSpace
    /// \return Dataspace associated with the memory representation of this
    /// selection
    ///
    DataSpace getMemSpace() const;

    ///
    /// \brief getDataSet
    /// \return parent dataset of this selection
    ///
    DataSet& getDataset();
    const DataSet& getDataset() const;

    ///
    /// \brief return the datatype of the selection
    /// \return return the datatype of the selection
    const DataType getDataType() const;

  protected:
    Selection(const DataSpace& memspace, const DataSpace& file_space, const DataSet& set);

  private:
    DataSpace _mem_space, _file_space;
    DataSet _set;

#if HIGHFIVE_HAS_FRIEND_DECLARATIONS
    template <typename Derivate>
    friend class ::HighFive::SliceTraits;
#endif
    friend Selection detail::make_selection(const DataSpace&, const DataSpace&, const DataSet&);
};

}  // namespace HighFive
