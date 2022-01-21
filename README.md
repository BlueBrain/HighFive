# HighFive - HDF5 header-only C++ Library

[![Build Status](https://travis-ci.org/BlueBrain/HighFive.svg?branch=master)](https://travis-ci.org/BlueBrain/HighFive)
[![Coverity Statys](https://scan.coverity.com/projects/13635/badge.svg)](https://scan.coverity.com/projects/highfive)
[![Doxygen -> gh-pages](https://github.com/BlueBrain/HighFive/workflows/gh-pages/badge.svg)](https://BlueBrain.github.io/HighFive)
[![codecov](https://codecov.io/gh/BlueBrain/HighFive/branch/master/graph/badge.svg?token=UBKxHEn7RS)](https://codecov.io/gh/BlueBrain/HighFive)

Documentation: https://bluebrain.github.io/HighFive/

## Brief

HighFive is a modern header-only C++11 friendly interface for libhdf5.

HighFive supports STL vector/string, Boost::UBLAS, Boost::Multi-array, Eigen and Xtensor. It handles C++ from/to HDF5 with automatic type mapping.
HighFive does not require additional libraries (see dependencies) and supports both HDF5 thread safety and Parallel HDF5 (contrary to the official hdf5 cpp)

It integrates nicely with other CMake projects by defining (and exporting) a HighFive target.


### Design
- Simple C++-ish minimalist interface
- No other dependency than libhdf5
- Zero overhead
- Support C++11

### Feature support
- create/read/write files, datasets, attributes, groups, dataspaces.
- automatic memory management / ref counting
- automatic conversion of `std::vector` and nested `std::vector` from/to any dataset with basic types
- automatic conversion of `std::string` to/from variable length string dataset
- selection() / slice support
- parallel Read/Write operations from several nodes with Parallel HDF5
- Advanced types: Compound, Enum, Arrays of Fixed-length strings, References
- etc... (see [ChangeLog](./CHANGELOG.md))

### Dependencies
- hdf5 (dev)
- hdf5-mpi (optional, opt-in with -D*HIGHFIVE_PARALLEL_HDF5*=ON)
- boost >= 1.41 (recommended, opt-out with -D*HIGHFIVE_USE_BOOST*=OFF)
- eigen3 (optional, opt-in with -D*HIGHFIVE_USE_EIGEN*=ON)
- xtensor (optional, opt-in with -D*HIGHFIVE_USE_XTENSOR*=ON)


## Examples

#### Write a std::vector<int> to 1D HDF5 dataset and read it back

```c++
#include <highfive/H5File.hpp>

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

**Note:** `H5File.hpp` is the top-level header of HighFive core which should be always included.

**Note:** If you can use `DataSpace::From` on your data, you can combine the create and write into one statement.
Such shortcut syntax is available for both `createDataSet` and `createAttribute`.
```c++
DataSet dataset = file.createDataSet("/dataset_one",  data);
```

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


### Compiling with HighFive

```bash
c++ -o program -I/path/to/highfive/include source.cpp  -lhdf5
```

### H5Easy

For several 'standard' use cases the [highfive/H5Easy.hpp](include/highfive/H5Easy.hpp) interface is available. It allows:

* Reading/writing in a single line of:

    - scalars (to/from an extendible DataSet),
    - strings,
    - vectors (of standard types),
    - [Eigen::Matrix](http://eigen.tuxfamily.org) (optional, enable CMake option `HIGHFIVE_USE_EIGEN`),
    - [xt::xarray](https://github.com/QuantStack/xtensor) and [xt::xtensor](https://github.com/QuantStack/xtensor)
      (optional, enable CMake option `HIGHFIVE_USE_XTENSOR`).
    - [cv::Mat_](https://docs.opencv.org/master/df/dfc/classcv_1_1Mat__.html)
      (optional, enable CMake option `HIGHFIVE_USE_OPENCV`).

* Getting in a single line:

    - the size of a DataSet,
    - the shape of a DataSet.

#### Example

```cpp
#include <highfive/H5Easy.hpp>

int main() {
    H5Easy::File file("example.h5", H5Easy::File::Overwrite);

    int A = ...;
    H5Easy::dump(file, "/path/to/A", A);

    A = H5Easy::load<int>(file, "/path/to/A");
}
```

whereby the `int` type of this example can be replaced by any of the above types. See [easy_load_dump.cpp](src/examples/easy_load_dump.cpp) for more details.

**Note:** Classes such as `H5Easy::File` are just short for the regular `HighFive` classes (in this case `HighFive::File`). They can thus be used interchangeably.


## CMake integration

HighFive can easily be used by other C++ CMake projects.

You may use HighFive from a folder in your project (typically a git submodule).
```cmake
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(foo)
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(highfive_folder)
add_executable(bar bar.cpp)
target_link_libraries(bar HighFive)
```

Alternativelly you can install HighFive once and use it in several projects via `find_package()`.

A HighFive target will bring the compilation settings to find HighFive headers and all chosen dependencies.

```cmake
# ...
find_package(HighFive REQUIRED)
add_executable(bar bar.cpp)
target_link_libraries(bar HighFive)
```
**Note:** Like with other libraries you may need to provide CMake the location to find highfive: `CMAKE_PREFIX_PATH=<highfive_install_dir>`

**Note:** `find_package(HighFive)` will search dependencies as well (e.g. Boost if requested). In order to use the same dependencies found at HighFive install time (e.g. for system deployments) you may set `HIGHFIVE_USE_INSTALL_DEPS=YES`

### Installing
When installing via CMake, besides the headers, a HighFiveConfig.cmake is generated which provides the HighFive target, as seen before. Note: You may need to set `CMAKE_INSTALL_PREFIX`:
```bash
mkdir build && cd build
# Look up HighFive CMake options, consider inspecting with `ccmake`
cmake .. -DHIGHFIVE_EXAMPLES=OFF -DCMAKE_INSTALL_PREFIX="<highfive_install_dir>"
make install
```

### Test Compilation
As a header-only library, HighFive doesn't require compilation. You may however build tests and examples.

```bash
mkdir build && cd build
cmake ../
make  # build tests and examples
make test  # build and run unit tests
```

**Note:** Unit tests require Boost. In case it's unavailable you may use `-DHIGHFIVE_USE_BOOST=OFF`.
HighFive with disable support for Boost types as well as unit tests (though most examples will build).


# Funding & Acknowledgment
 
The development of this software was supported by funding to the Blue Brain Project, a research center of the École polytechnique fédérale de Lausanne (EPFL), from the Swiss government's ETH Board of the Swiss Federal Institutes of Technology.
 
Copyright © 2015-2021 Blue Brain Project/EPFL


### License

Boost Software License 1.0
