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

#include "H5FileDriver.hpp"
#include "H5Object.hpp"
#include "bits/H5Annotate_traits.hpp"
#include "bits/H5Node_traits.hpp"

namespace HighFive {

///
/// \brief File class
///
class File : public Object,
             public NodeTraits<File>,
             public AnnotateTraits<File> {
 public:

    const static ObjectType type = ObjectType::File;

    enum : unsigned {
        /// Open flag: Read only access
        ReadOnly = 0x00u,
        /// Open flag: Read Write access
        ReadWrite = 0x01u,
        /// Open flag: Truncate a file if already existing
        Truncate = 0x02u,
        /// Open flag: Open will fail if file already exist
        Excl = 0x04u,
        /// Open flag: Open in debug mode
        Debug = 0x08u,
        /// Open flag: Create non existing file
        Create = 0x10u,
        /// Derived open flag: common write mode (=ReadWrite|Create|Truncate)
        Overwrite = Truncate,
        /// Derived open flag: Opens RW or exclusively creates
        OpenOrCreate = ReadWrite | Create
    };

    ///
    /// \brief File
    /// \param filename: filepath of the HDF5 file
    /// \param openFlags: Open mode / flags ( ReadOnly, ReadWrite)
    /// \param fileAccessProps: the file access properties
    ///
    /// Open or create a new HDF5 file
    explicit File(const std::string& filename, unsigned openFlags = ReadOnly,
                  const FileAccessProps& fileAccessProps = FileAccessProps::Default());

    ///
    /// \brief Return the name of the file
    ///
    const std::string& getName() const noexcept;


    /// \brief Object path of a File is always "/"
    std::string getPath() const noexcept {
        return "/";
    }

    ///
    /// \brief flush
    ///
    /// Flushes all buffers associated with a file to disk
    ///
    void flush();

 private:
    using Object::Object;

    mutable std::string _filename{};

    template <typename> friend class PathTraits;
};

}  // namespace HighFive

// H5File is the main user constructible -> bring in implementation headers
#include "bits/H5Annotate_traits_misc.hpp"
#include "bits/H5File_misc.hpp"
#include "bits/H5Node_traits_misc.hpp"
#include "bits/H5Path_traits_misc.hpp"

#endif  // H5FILE_HPP
