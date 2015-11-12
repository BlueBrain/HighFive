#ifndef H5FILE_HPP
#define H5FILE_HPP

#include <string>

#include "H5Object.hpp"
#include "bits/H5Node_traits.hpp"


namespace HighFive{



class File : public Object, public NodeTraits<File> {
public:

    /// Open flag: Read only access
    static const int ReadOnly;
    /// Open flag: Read Write access
    static const int ReadWrite;
    /// Open flag: Create non existing file
    static const int Create;
    /// Open flag: Truncate a file if already existing
    static const int Truncate;

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
