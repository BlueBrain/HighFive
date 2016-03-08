/*
 * Copyright (C) 2015 Adrien Devresse <adrien.devresse@epfl.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#ifndef H5SELECTION_HPP
#define H5SELECTION_HPP

#include "H5Object.hpp"
#include "H5DataSet.hpp"

#include "bits/H5Slice_traits.hpp"

namespace HighFive {

template<typename Derivate> class SliceTraits;

///
/// \brief Selection: represent a view on a slice/part of a dataset
///
/// A Selection is valid only if its parent dataset is valid
///
class Selection : public SliceTraits<Selection>
{
public:

    ///
    /// \brief getSpace
    /// \return Dataspace associated with this selection
    ///
    DataSpace getSpace() const;


    ///
    /// \brief getMemSpace
    /// \return Dataspace associated with the memory representation of this selection
    ///
    DataSpace getMemSpace() const;

    ///
    /// \brief getDataSet
    /// \return parent dataset of this selection
    ///
    DataSet & getDataset();
    const DataSet & getDataset() const;

private:
    Selection(const DataSpace & memspace, const DataSpace & file_space, const DataSet & set);

    DataSpace _mem_space, _file_space;
    DataSet _set;

    template<typename Derivate>
    friend class ::HighFive::SliceTraits;
    // absolute namespace naming due to GCC bug 52625

};

}

#include "bits/H5Selection_misc.hpp"


#endif // H5SELECTION_HPP
