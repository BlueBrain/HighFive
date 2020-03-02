# HighFive - HDF5 header-only C++ Library

[![Build Status](https://travis-ci.org/BlueBrain/HighFive.svg?branch=master)](https://travis-ci.org/BlueBrain/HighFive)

[![Coverity Statys](https://scan.coverity.com/projects/13635/badge.svg)](https://scan.coverity.com/projects/highfive)

## Brief

HighFive is a modern C++/C++11 friendly interface for libhdf5.

HighFive supports STL vector/string, Boost::UBLAS and Boost::Multi-array. It handles C++ from/to HDF5 automatic type mapping.
HighFive does not require an additional library and supports both HDF5 thread safety and Parallel HDF5 (contrary to the official hdf5 cpp)


### Design

- Simple C++-ish minimalist interface
- No other dependency than libhdf5
- Zero overhead
- Support C++11


### Dependencies
- hdf5 (dev)
- hdf5-mpi (optional, opt-in with -D*DHIGHFIVE_PARALLEL_HDF5*=ON)
- boost >= 1.41 (recommended, opt-out with -D*DHIGHFIVE_USE_BOOST*=OFF)
- eigen3 (optional, opt-in with -D*HIGHFIVE_USE_EIGEN*=ON)
- xtensor (optional, opt-in with -D*HIGHFIVE_USE_XTENSOR*=ON)


### CMake integration

HighFive can easily be used by other C++ CMake projects.
Below is a very simple *foo* project creating a *bar* C++ program
using HighFive library:

```cmake
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(foo)
set(CMAKE_CXX_STANDARD 11)

find_package(HighFive 2.1 REQUIRED)
add_executable(bar bar.cpp)
target_link_libraries(bar HighFive)
```

### Usage

#### Write a std::vector<int> to 1D HDF5 dataset and read it back

```c++
using namespace HighFive;
// we create a new hdf5 file
File file("/tmp/new_file.h5", File::ReadWrite | File::Create | File::Truncate);

std::vector<int> data(50, 1);

// let's create a dataset of native integer with the size of the vector 'data'
DataSet dataset = file.createDataSet<int>("/dataset_one",  DataSpace::From(data));

// let's write our vector of int to the HDF5 dataset
dataset.write(data);

// read back
std::vector<int> result;
dataset.read(result);
```

> Note: if you can use `DataSpace::From` on your data, you can combine the create and write into one statement:
> 
> ```c++
> DataSet dataset = file.createDataSet("/dataset_one",  data);
> ```
>
> This works with `createAttribute`, as well.

#### Write a 2 dimensional C double float array to a 2D HDF5 dataset

See [create_dataset_double.cpp](src/examples/create_dataset_double.cpp)

#### Write and read a matrix of double float (boost::ublas) to a 2D HDF5 dataset

See [boost_ublas_double.cpp](src/examples/boost_ublas_double.cpp)

#### Write and read a subset of a 2D double dataset

See [select_partial_dataset_cpp11.cpp](src/examples/select_partial_dataset_cpp11.cpp)

#### Create, write and list HDF5 attributes

See [create_attribute_string_integer.cpp](src/examples/create_attribute_string_integer.cpp)

#### And others

See [src/examples/](src/examples/) subdirectory for more info.

### Compile with HighFive

```bash
c++ -o program -I/path/to/highfive/include source.cpp  -lhdf5
```

#### H5Easy

For several 'standard' use cases the [highfive/H5Easy.hpp](include/highfive/H5Easy.hpp) interface is available. It allows:

*   Reading/writing in a single line of:

    -   scalars (to/from an extendible DataSet),
    -   strings,
    -   vectors (of standard types),
    -   [Eigen::Matrix](http://eigen.tuxfamily.org) (optional, enable CMake option `HIGHFIVE_USE_EIGEN`),
    -   [xt::xarray](https://github.com/QuantStack/xtensor) and [xt::xtensor](https://github.com/QuantStack/xtensor)
        (optional, enable CMake option `HIGHFIVE_USE_XTENSOR`).

*   Getting in a single line:

     -   the size of a DataSet,
     -   the shape of a DataSet.

The general idea is to 

```cpp
#include <highfive/H5Easy.hpp>

int main()
{
    H5Easy::File file("example.h5", H5Easy::File::Overwrite);

    int A = ...;

    H5Easy::dump(file, "/path/to/A", A);

    A = H5Easy::load<int>(file, "/path/to/A");
}
```

whereby the `int` type of this example can be replaced by any of the above types. See [easy_load_dump.cpp](src/examples/easy_load_dump.cpp) for more details.

> Note that classes such as `H5Easy::File` are just short for the regular `HighFive` classes (in this case `HighFive::File`). They can thus be used interchangeably.

### Test Compilation
Remember: Compilation is not required. Used only for unit test and examples
Unit tests need boost (*USE_BOOST*).

```bash
mkdir build; pushd build
cmake ../
make
make test
```

### Feature support

- create/read/write file, dataset, group, dataspace.
- automatic memory management / ref counting
- automatic conversion of `std::vector` and nested `std::vector` from/to any dataset with basic types
- automatic conversion of `std::string` to/from variable length string dataset
- selection() / slice support
- parallel Read/Write operations from several nodes with Parallel HDF5
- support HDF5 attributes


### Contributors

- Adrien Devresse <adrien.devresse@epfl.ch> - Blue Brain Project
- Ali Can Demiralp <demiralpali@gmail.com> -
- Fernando Pereira <fernando.pereira@epfl.ch> - Blue Brain Project
- Stefan Eilemann <stefan.eilemann@epfl.ch> - Blue Brain Project
- Tristan Carel <tristan.carel@epfl.ch> - Blue Brain Project
- Wolf Vollprecht <w.vollprecht@gmail.com> - QuantStack
- Tom de Geus <tom@geus.me> - EPFL
- Nicolas Cornu <nicolas.cornu@epfl.ch> - Blue Brain Project

### License

Boost Software License 1.0
