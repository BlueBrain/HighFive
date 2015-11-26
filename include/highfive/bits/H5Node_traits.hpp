#ifndef H5NODE_TRAITS_HPP
#define H5NODE_TRAITS_HPP

#include <string>


namespace HighFive{

class DataSet;
class Group;
class DataSpace;
class DataType;

template <typename Derivate>
class NodeTraits{
public:
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
    /// \brief get an existing dataset in the current file
    /// \param dataset_name
    /// \return return the named dataset, or throw exception if not found
    ///
    DataSet getDataSet(const std::string & dataset_name);

    ///
    /// \brief create a new group with the name group_name
    /// \param group_name
    /// \return the group object
    ///
    Group createGroup(const std::string & group_name);

    ///
    /// \brief open an existing group with the name group_name
    /// \param group_name
    /// \return the group object
    ///
    Group getGroup(const std::string & group_name);

private:
    typedef Derivate derivate_type;


};


}

#include "H5Node_traits_misc.hpp"

#endif // H5NODE_TRAITS_HPP
