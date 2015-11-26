#ifndef H5DATASPACE_HPP
#define H5DATASPACE_HPP

#include <vector>

#include "H5Object.hpp"



namespace HighFive{

class File;
class DataSet;

class DataSpace : public Object{
public:
    /// create a dataspace of N-dimensions
    /// Each dimension is configured this way
    ///  size(dim1) = vec[0]
    ///  size(dim2) = vec[1]
    ///  etc...
    explicit DataSpace(const std::vector<size_t> & dims);

    ///
    /// \brief DataSpace create a dataspace of a single dimension and of size dim1
    /// \param dim1
    ///
    explicit DataSpace(size_t dim1);

    ///
    /// \brief getNumberDimensions
    /// \return the number of dimensions in the current dataspace
    ///
    size_t getNumberDimensions() const;

    /// \brief getDimensions
    /// \return return a vector of N-element, each element is the size of the associated dataset dimension
    std::vector<size_t> getDimensions() const ;

    /// Create a dataspace matching the container dimensions and size
    /// Supported Containers are:
    ///  - vector of fundamental types
    ///  - vector of std::string
    template<typename Container>
    static DataSpace From(const Container & vec);

protected:

    explicit DataSpace();

    friend class File;
    friend class DataSet;
};

}

#include "bits/H5Dataspace_misc.hpp"

#endif // H5DATASPACE_HPP
