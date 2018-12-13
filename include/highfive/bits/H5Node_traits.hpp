/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5NODE_TRAITS_HPP
#define H5NODE_TRAITS_HPP

#include <string>

namespace HighFive {

class Attribute;
class DataSet;
class Group;
class DataSpace;
class DataType;

template <typename Derivate>
class NodeTraits {
  public:
    ///
    /// \brief createDataSet Create a new dataset in the current file of
    /// datatype type and of size space
    /// \param dataset_name identifier of the dataset
    /// \param space Associated DataSpace, see \ref DataSpace for more
    /// informations
    /// \param type Type of Data
    /// \param createProps A property list with data set creation properties
    /// \param createProps A property list with data set access properties
    /// \return DataSet Object
    DataSet
    createDataSet(const std::string& dataset_name,
                  const DataSpace& space,
                  const DataType& type,
                  const DataSetCreateProps& createProps = DataSetCreateProps(),
                  const DataSetAccessProps& accessProps = DataSetAccessProps());

    ///
    /// \brief createDataSet create a new dataset in the current file with a
    /// size specified by space
    /// \param dataset_name identifier of the dataset
    /// \param space Associated DataSpace, see \ref DataSpace for more
    /// information
    /// \param createProps A property list with data set creation properties
    /// \return DataSet Object
    template <typename Type>
    DataSet
    createDataSet(const std::string& dataset_name,
                  const DataSpace& space,
                  const DataSetCreateProps& createProps = DataSetCreateProps(),
                  const DataSetAccessProps& accessProps = DataSetAccessProps());

    ///
    /// \brief createDataSet create a new dataset in the current file and
    /// write to it, inferring the DataSpace from the data.
    /// \param dataset_name identifier of the dataset
    /// \param data Associated data, must support DataSpace::From, see
    /// \ref DataSpace for more information
    /// \param createProps A property list with data set creation properties
    /// \return DataSet Object
    template <typename T>
    DataSet
    createDataSet(const std::string& dataset_name,
                  const T& data,
                  const DataSetCreateProps& createProps = DataSetCreateProps(),
                  const DataSetAccessProps& accessProps = DataSetAccessProps());

    ///
    /// \brief get an existing dataset in the current file
    /// \param dataset_name
    /// \param accessProps property list to configure dataset chunk cache
    /// \return return the named dataset, or throw exception if not found
    DataSet getDataSet(
        const std::string& dataset_name,
        const DataSetAccessProps& accessProps = DataSetAccessProps()) const;

    ///
    /// \brief create a new group, and eventually intermediate groups
    /// \param group_name
    /// \param recursive Whether it shall create intermediate groups if
    //       necessary. Default: true
    /// \return the group object
    Group createGroup(const std::string& group_name, bool parents = true);

    ///
    /// \brief open an existing group with the name group_name
    /// \param group_name
    /// \return the group object
    Group getGroup(const std::string& group_name) const;

    ///
    /// \brief return the number of leaf objects of the node / group
    /// \return number of leaf objects
    size_t getNumberObjects() const;

    ///
    /// \brief return the name of the object with the given index
    /// \return the name of the object
    std::string getObjectName(size_t index) const;

    ///
    /// \brief list all leaf objects name of the node / group
    /// \return number of leaf objects
    std::vector<std::string> listObjectNames() const;

    ///
    /// \brief check a dataset or group exists in the current node / group
    /// \param dataset/group name to check
    /// \return true if a dataset/group with the asssociated name exist, or
    /// false
    bool exist(const std::string& node_name) const;

  private:
    typedef Derivate derivate_type;

    // A wrapper over the low-level H5Lexist
    bool _exist(const std::string& node_name) const;
};

}  // namespace HighFive

#include "H5Node_traits_misc.hpp"

#endif  // H5NODE_TRAITS_HPP
