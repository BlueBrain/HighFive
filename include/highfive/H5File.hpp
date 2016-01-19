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
#ifndef H5FILE_HPP
#define H5FILE_HPP

#include <string>

#include "H5Object.hpp"
#include "bits/H5Node_traits.hpp"


namespace HighFive{



class File : public Object, public NodeTraits<File> {
public:

    /// Open flag: Read only access
    static const int ReadOnly=0x00;
    /// Open flag: Read Write access
    static const int ReadWrite=0x01;
    /// Open flag: Truncate a file if already existing
    static const int Truncate=0x02;
    /// Open flag: Open will fail if file already exist
    static const int Excl=0x04;
    /// Open flag: Open in debug mode
    static const int Debug=0x08;
    /// Open flag: Create non existing file
    static const int Create=0x10;



    ///
    /// \brief File
    /// \param filename: filepath of the HDF5 file
    /// \param openFlags: Open mode / flags ( ReadOnly, ReadWrite)
    ///
    /// Open or create a new HDF5 file
    explicit File(const std::string & filename, int openFlags = ReadOnly);

    virtual ~File(){}



private:
    std::string _filename;
};

}

#include "bits/H5File_misc.hpp"

#endif // H5FILE_HPP
