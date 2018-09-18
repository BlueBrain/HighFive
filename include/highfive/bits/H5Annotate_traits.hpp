/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5ANNOTATE_TRAITS_HPP
#define H5ANNOTATE_TRAITS_HPP

#include <string>

namespace HighFive {

class Attribute;
class DataSet;
class Group;
class DataSpace;
class DataType;

template <typename Derivate>
class AnnotateTraits {
  public:
    ///
    /// \brief create a new attribute with the name attribute_name
    /// \param attribute_name
    /// \return the attribute object
    ///
    Attribute createAttribute(const std::string& attribute_name,
                              const DataSpace& space, const DataType& type);

    ///
    /// \brief createAttribute create a new attribute on the current dataset with
    /// size specified by space
    /// \param dataset_name identifier of the attribute
    /// \param space Associated DataSpace, see \ref DataSpace for more
    /// informations
    /// \return Attribute Object
    ///
    ///
    ///
    template <typename Type>
    Attribute createAttribute(const std::string& attribute_name,
                              const DataSpace& space);

    ///
    /// \brief createAttribute create a new attribute on the current dataset and
    /// write to it, inferring the DataSpace from data.
    /// \param attribute_name identifier of the attribute
    /// \param data Associated data to write, must support DataSpace::From, see
    ///  \ref DataSpace for more information
    /// \return Attribute Object
    ///
    template <typename T>
    Attribute createAttribute(const std::string& attribute_name,
                              const T& data);

    ///
    /// \brief open an existing attribute with the name attribute_name
    /// \param attribute_name
    /// \return the attribute object
    ///
    Attribute getAttribute(const std::string& attribute_name) const;

    ///
    /// \brief return the number of attributes of the node / group
    /// \return number of attributes
    size_t getNumberAttributes() const;

    ///
    /// \brief list all attribute name of the node / group
    /// \return number of attributes
    std::vector<std::string> listAttributeNames() const;

    ///
    /// \brief checks an attribute exists
    /// \return number of attributes
    bool hasAttribute(const std::string& attr_name) const;

  private:
    typedef Derivate derivate_type;
};
}

#include "H5Annotate_traits_misc.hpp"

#endif // H5ANNOTATE_TRAITS_HPP
