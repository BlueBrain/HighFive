#ifndef H5DATASPACE_HPP
#define H5DATASPACE_HPP

#include <vector>

#include "H5Object.hpp"



namespace HighFive{

class File;
class DataSet;

class DataSpace : public Object{
public:
    explicit DataSpace(const std::vector<size_t> & dims);

    size_t getDims() const;

protected:

    explicit DataSpace();

    friend class File;
    friend class DataSet;
};

}

#include "bits/H5Dataspace_misc.hpp"

#endif // H5DATASPACE_HPP
