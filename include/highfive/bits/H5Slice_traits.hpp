#ifndef H5SLICE_TRAITS_HPP
#define H5SLICE_TRAITS_HPP

namespace HighFive{

class DataSet;
class Group;
class DataSpace;
class DataType;

template<typename Derivate>
class SliceTraits{
public:

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
    typedef Derivate derivate_type;
};

}

#include "H5Slice_traits_misc.hpp"

#endif // H5SLICE_TRAITS_HPP
