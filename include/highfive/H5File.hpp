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
    /// \brief createDataSet Create a new dataset in the current file of datatype type and of size space
    /// \param dataset_name identifier of the dataset
    /// \param space Associated DataSpace, see \ref DataSpace for more informations
    /// \param type Type of Data
    /// \return DataSet Object
    DataSet createDataSet(const std::string & dataset_name, const DataSpace & space, const DataType & type);

    ///
    /// \brief createDataSet create a new dataset in the current file with a size specified by space
    /// \param dataset_name identifier of the dataset
    /// \param space Associated DataSpace, see \ref DataSpace for more informations
    /// \return DataSet Object
    ///
    ///
    ///
    template <typename Type>
    DataSet createDataSet(const std::string & dataset_name, const DataSpace & space);

    ///
    /// \brief createDataSet create a new dataset of the size of the associated Vector
    /// \param dataset_name identifier of the dataset
    /// \return DataSet Object
    template <typename Vector>
    DataSet createDataSet(const std::string & dataset_name, const Vector & vector);

private:
    std::string _filename;
};

}

#include "bits/H5File_misc.hpp"

#endif // H5FILE_HPP
