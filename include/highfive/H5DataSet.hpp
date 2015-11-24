#ifndef H5DATASET_HPP
#define H5DATASET_HPP

#include <vector>

#include "H5Object.hpp"




namespace HighFive{


template <typename Derivate> class NodeTraits;
class DataType;
class DataSpace;

class DataSet : public Object{
public:

    size_t getStorageSize() const;

    ///
    /// \brief getDataType
    /// \return return the datatype associated with this dataset
    ///
    DataType getDataType() const;

    ///
    /// \brief getSpace
    /// \return return the dataspace associated with this dataset
    ///
    DataSpace getSpace() const;




    ///
    /// Read the entire dataset into a buffer
    /// An exception is raised is if the numbers of dimension of the buffer and of the dataset are different
    ///
    /// The array type can be a N-pointer or a N-vector ( e.g int** integer two dimensional array )
    template <typename T>
    void read(T & array);

    ///
    /// Write the integrality N-dimension buffer to this dataset
    /// An exception is raised is if the numbers of dimension of the buffer and of the dataset are different
    ///
    /// The array type can be a N-pointer or a N-vector ( e.g int** integer two dimensional array )
    template <typename T>
    void write(T & buffer);

private:
    DataSet();
    template <typename Derivate> friend class NodeTraits;

};

}

#include "bits/H5DataSet_misc.hpp"

#endif // H5DATASET_HPP
