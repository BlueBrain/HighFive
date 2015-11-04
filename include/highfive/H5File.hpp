#ifndef H5FILE_HPP
#define H5FILE_HPP

#include <string>

#include "H5Object.hpp"


namespace HighFive{

class DataSet;
class DataSpace;
class DataType;

class File : public Object {
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

    ///
    /// \brief createDataSet Create a new DataSet in the current file
    /// \param dataset_name identifier of the dataset
    /// \param space Associated DataSpace, see \ref DataSpace for more informations
    /// \param type Type of Data
    /// \return DataSet Object
    DataSet createDataSet(const std::string & dataset_name, const DataSpace & space, const DataType & type);

    ///
    /// \brief createDataSet Create a new DataSet in the current file with a basic datatype T ( e.g T = int, double )
    /// \param dataset_name identifier of the dataset
    /// \param space Associated DataSpace, see \ref DataSpace for more informations
    /// \return DataSet Object
    template <typename Type>
    DataSet createDataSet(const std::string & dataset_name, const DataSpace & space);

private:
    std::string _filename;
};

}

#include "bits/H5File_misc.hpp"

#endif // H5FILE_HPP
