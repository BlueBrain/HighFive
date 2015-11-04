#ifndef H5DATASET_HPP
#define H5DATASET_HPP

#include <vector>

#include "H5Object.hpp"



namespace HighFive{

class File;
class DataType;
class DataSpace;

class DataSet : public Object{
public:

    size_t getStorageSize() const;

    DataType getDataType() const;

    DataSpace getSpace() const;




    ///
    /// Read the entire dataset into a buffer
    /// An exception is raised is if the numbers of dimension of the buffer and of the dataset are different
    ///
    /// The array type can be a N-pointer or a N-vector ( e.g int** integer two dimensional array )
    template <typename T>
    void read(T array);

    ///
    /// Write the integrality N-dimension buffer to this dataset
    /// An exception is raised is if the numbers of dimension of the buffer and of the dataset are different
    ///
    /// The array type can be a N-pointer or a N-vector ( e.g int** integer two dimensional array )
    template <typename T>
    void write(T buffer);

protected:
    DataSet();

    friend class File;
};

}

#include "bits/H5DataSet_misc.hpp"

#endif // H5DATASET_HPP
