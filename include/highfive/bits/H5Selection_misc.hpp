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
#ifndef H5SELECTION_MISC_HPP
#define H5SELECTION_MISC_HPP

#include "../H5Selection.hpp"

namespace HighFive{


Selection::Selection(const DataSpace & memspace, const DataSpace & file_space, DataSet & set) :
    _mem_space(memspace),
    _file_space(file_space),
    _set(set){

}


DataSpace Selection::getSpace() const{
    return _file_space;
}

DataSpace Selection::getMemSpace() const{
    return _mem_space;
}

DataSet & Selection::getDataset(){
    return _set;
}

}


#endif // H5SELECTION_MISC_HPP
