/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5FILE_HPP
#define H5FILE_HPP

#include <string>

#include "H5Object.hpp"
#include "H5FileDriver.hpp"

#include "bits/H5Node_traits.hpp"
#include "bits/H5Annotate_traits.hpp"


namespace HighFive{

///
/// \brief File class
///
class File : public Object, public NodeTraits<File>, public AnnotateTraits<File> {
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
    explicit File(const std::string & filename, int openFlags = ReadOnly, const FileDriver & driver = default_file_driver());

    virtual ~File();


    void flush();

    bool hasGroup(const char * name);



private:
    std::string _filename;
};




}

#include "bits/H5File_misc.hpp"

#endif // H5FILE_HPP
