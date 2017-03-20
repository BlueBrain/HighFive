HighFive - HDF5 header-only C++ Library

# Brief

HighFive is a modern C++/C++11 friendly interface for libhdf5. 

HighFive supports STL vector/string, Boost::UBLAS and Boost::Multi-array. It handles C++ from/to HDF5 automatic type mapping. 
HighFive does not require an additional library and supports both HDF5 thread safety and Parallel HDF5 (contrary to the official hdf5 cpp)


## Design
- Simple C++-ish minimalist interface
- No other dependency than libhdf5  
- Zero overhead
- Support C++11 ( compatible with C++98 )


## Dependencies
- libhdf5
- (optional) boost >= 1.41 


## Usage

```c++
    using namespace HighFive;
    // we create a new hdf5 file
    File file("/tmp/new_hdf5_file.h5", File::ReadWrite | File::Create | File::Truncate);

    std::vector<int> data(50, 1);

    // lets create a dataset of native interger with the size of the vector 'data'
    DataSet dataset = file.createDataSet<int>("/dataset_one",  DataSpace::From(data));

    // lets write our vector of int to the HDF5 dataset
    dataset.write(data);
 
    // read back
    std::vector<int> result
    dataset.read(result);
    
```

See examples/  sub-directory for more infos

## Compile with HighFive

c++ -o program -I/path/to/highfive/include source.cpp  -lhdf5


## Test Compilation
Remember: Compilation is not required. Used only for unit test and examples

mkdir build; pushd build   
cmake ../   
make   
make test   


## Feature support
    - Create/read/write file,  dataset, group, dataspace.
    - Automatic memory management / ref counting
    - Automatic convertion of  std::vector and nested std::vector from/to any dataset with basic types
    - Automatic convertion of std::string to/from variable length string dataset
    - selection() / slice support
    - support HDF5 attributes


## Contributors
Adrien Devresse <adrien.devresse@epfl.ch> - Blue Brain Project   
Ali Can Demiralp <demiralpali@gmail.com>   
Stefan Eilemann <stefan.eilemann@epfl.ch> - Blue Brain Project  


## License
Boost Software License 1.0 





