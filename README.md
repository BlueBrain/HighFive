# HighFive - HDF5 header-only C++ Library

[![Doxygen -> gh-pages](https://github.com/BlueBrain/HighFive/workflows/gh-pages/badge.svg)](https://BlueBrain.github.io/HighFive)
[![codecov](https://codecov.io/gh/BlueBrain/HighFive/branch/master/graph/badge.svg?token=UBKxHEn7RS)](https://codecov.io/gh/BlueBrain/HighFive)
[![HighFive_Integration_tests](https://github.com/BlueBrain/HighFive-testing/actions/workflows/integration.yml/badge.svg)](https://github.com/BlueBrain/HighFive-testing/actions/workflows/integration.yml)

Documentation: https://bluebrain.github.io/HighFive/

## Brief

HighFive is a modern header-only C++11 friendly interface for libhdf5.

HighFive supports STL vector/string, Boost::UBLAS, Boost::Multi-array and Xtensor. It handles C++ from/to HDF5 with automatic type mapping.
HighFive does not require additional libraries (see dependencies).

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
- half-precision (16-bit) floating-point datasets
- `std::byte` in C++17 mode (with `-DCMAKE_CXX_STANDARD=17` or higher)
- etc... (see [ChangeLog](./CHANGELOG.md))

### Dependencies
- hdf5 (dev)
- hdf5-mpi (optional, opt-in with -D*HIGHFIVE_PARALLEL_HDF5*=ON)
- boost >= 1.41 (recommended, opt-out with -D*HIGHFIVE_USE_BOOST*=OFF)
- eigen3 (optional, opt-in with -D*HIGHFIVE_USE_EIGEN*=ON)
- xtensor (optional, opt-in with -D*HIGHFIVE_USE_XTENSOR*=ON)
- half (optional, opt-in with -D*HIGHFIVE_USE_HALF_FLOAT*=ON)

### Known flaws
- HighFive is not thread-safe. At best it has the same limitations as the HDF5 library. However, HighFive objects modify their members without protecting these writes. Users have reported that HighFive is not thread-safe even when using the threadsafe HDF5 library, e.g., https://github.com/BlueBrain/HighFive/discussions/675.
- Eigen support in core HighFive is broken. See https://github.com/BlueBrain/HighFive/issues/532. H5Easy is not
  affected.
- The support of fixed length strings isn't ideal.


## Examples

#### Write a std::vector<int> to 1D HDF5 dataset and read it back

```c++
#include <highfive/highfive.hpp>

using namespace HighFive;

std::string filename = "/tmp/new_file.h5";

{
    // We create an empty HDF55 file, by truncating an existing
    // file if required:
    File file(filename, File::Truncate);

    std::vector<int> data(50, 1);
    file.createDataSet("grp/data", data);
}

{
    // We open the file as read-only:
    File file(filename, File::ReadOnly);
    auto dataset = file.getDataSet("grp/data");

    // Read back, with allocating:
    auto data = dataset.read<std::vector<int>>();

    // Because `data` has the correct size, this will
    // not cause `data` to be reallocated:
    dataset.read(data);
}
```

**Note:** `H5File.hpp` is the top-level header of HighFive core which should be always included.

**Note:** For advanced usecases the dataset can be created without immediately
writing to it. This is common in MPI-IO related patterns, or when growing a
dataset over the course of a simulation.

#### Write a 2 dimensional C double float array to a 2D HDF5 dataset

See [create_dataset_double.cpp](https://github.com/BlueBrain/HighFive/blob/master/src/examples/create_dataset_double.cpp)

#### Write and read a matrix of double float (boost::ublas) to a 2D HDF5 dataset

See [boost_ublas_double.cpp](https://github.com/BlueBrain/HighFive/blob/master/src/examples/boost_ublas_double.cpp)

#### Write and read a subset of a 2D double dataset

See [select_partial_dataset_cpp11.cpp](https://github.com/BlueBrain/HighFive/blob/master/src/examples/select_partial_dataset_cpp11.cpp)

#### Create, write and list HDF5 attributes

See [create_attribute_string_integer.cpp](https://github.com/BlueBrain/HighFive/blob/master/src/examples/create_attribute_string_integer.cpp)

#### And others

See [src/examples/](https://github.com/BlueBrain/HighFive/blob/master/src/examples/) subdirectory for more info.


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
There's two common paths of integrating HighFive into a CMake based project.
The first is to "vendor" HighFive, the second is to install HighFive as a
normal C++ library. Due to how HighFive CMake code works, sometimes following
the third Bailout Approach is needed.

### Vendoring HighFive

In this approach the HighFive sources are included in a subdirectory of the
project (typically as a git submodule), for example in `third_party/HighFive`.

The projects `CMakeLists.txt` add the following lines
```cmake
add_executable(foo foo.cpp)

# You might want to turn off Boost support:
if(NOT DEFINED HIGHFIVE_USE_BOOST)
  set(HIGHFIVE_USE_BOOST Off)
endif()

# Include the subdirectory and use the target HighFive.
add_subdirectory(third_party/HighFive)
target_link_libraries(foo HighFive)
```

**Note:** `add_subdirectory(third_party/HighFive)` will search and "link" HDF5
and optional dependencies such as Boost.

### Regular Installation of HighFive

Alternatively you can install HighFive once and use it in several projects via
`find_package()`. First one should clone the sources:
```bash
git clone --recursive https://github.com/BlueBrain/HighFive.git HighFive-src
```
By default CMake will install systemwide, which is likely not appropriate. The
instruction below allow users to select a custom path where HighFive will be
installed, e.g. `HIGHFIVE_INSTALL_PREFIX=${HOME}/third_party/HighFive` or some
other location. The CMake invocations would be
```bash
cmake -DHIGHFIVE_EXAMPLES=Off \
      -DHIGHFIVE_USE_BOOST=Off \
      -DHIGHFIVE_UNIT_TESTS=Off \
      -DCMAKE_INSTALL_PREFIX=${HIGHFIVE_INSTALL_PREFIX} \
      -B HighFive-src/build \
      HighFive-src

cmake --build HighFive-src/build
cmake --install HighFive-src/build
```
This will install (i.e. copy) the headers to
`${HIGHFIVE_INSTALL_PREFIX}/include` and some CMake files into an appropriate
subfolder of `${HIGHFIVE_INSTALL_PREFIX}`.

The projects `CMakeLists.txt` should add the following:
```cmake
# ...
add_executable(foo foo.cpp)

find_package(HighFive REQUIRED)
target_link_libraries(foo HighFive)
```

**Note:** If HighFive hasn't been installed in a default location, CMake needs
to be told where to find it which can be done by adding
`-DCMAKE_PREFIX_PATH=${HIGHFIVE_INSTALL_PREFIX}` to the CMake command for
building the project using HighFive. The variable `CMAKE_PREFIX_PATH` is a
semi-colon `;` separated list of directories.

**Note:** `find_package(HighFive)` will search and "link" HDF5 and optional
dependencies such as Boost.

### The Bailout Approach
Since both `add_subdirectory` and `find_package` will trigger finding HDF5 and
other optional dependencies of HighFive as well as the `target_link_libraries`
code for "linking" with the dependencies, things can go wrong.

Fortunately, HighFive is a header only library and all that's needed is the
headers. Preferably, the version obtained by installing HighFive, since those
include `H5Version.hpp`. Let's assume they've been copied to
`third_party/HighFive`. Then one could create a target:

```bash
add_library(HighFive INTERFACE)
target_include_directory(HighFive INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/HighFive/include)


add_executable(foo foo.cpp)
target_link_libraries(foo HighFive)
```

One known case where this is required is when vendoring the optional
dependencies of HighFive.

# Questions?

Do you have questions on how to use HighFive? Would you like to share an interesting example or
discuss HighFive features? Head over to the [Discussions](https://github.com/BlueBrain/HighFive/discussions)
forum and join the community.

For bugs and issues please use [Issues](https://github.com/BlueBrain/HighFive/issues).

# Funding & Acknowledgment
 
The development of this software was supported by funding to the Blue Brain Project, a research center of the École polytechnique fédérale de Lausanne (EPFL), from the Swiss government's ETH Board of the Swiss Federal Institutes of Technology.
 
Copyright © 2015-2022 Blue Brain Project/EPFL


### License

Boost Software License 1.0
