*Note:* In preparation of `v3` of HighFive, we've started merging breaking
changes into the main branch. More information and opportunity to comment can
be found at:
https://github.com/BlueBrain/HighFive/issues/864

# HighFive - HDF5 header-only C++ Library

[![Doxygen -> gh-pages](https://github.com/BlueBrain/HighFive/workflows/gh-pages/badge.svg?branch=master)](https://BlueBrain.github.io/HighFive/actions/workflows/gh-pages.yml?query=branch%3Amaster)
[![codecov](https://codecov.io/gh/BlueBrain/HighFive/branch/master/graph/badge.svg?token=UBKxHEn7RS)](https://codecov.io/gh/BlueBrain/HighFive)
[![HighFive_Integration_tests](https://github.com/BlueBrain/HighFive-testing/actions/workflows/integration.yml/badge.svg)](https://github.com/BlueBrain/HighFive-testing/actions/workflows/integration.yml)
[![Zenodo](https://zenodo.org/badge/47755262.svg)](https://zenodo.org/doi/10.5281/zenodo.10679422)

Documentation: https://bluebrain.github.io/HighFive/

## Brief

HighFive is a modern header-only C++14 friendly interface for libhdf5.

HighFive supports STL vector/string, Boost::UBLAS, Boost::Multi-array and Xtensor. It handles C++ from/to HDF5 with automatic type mapping.
HighFive does not require additional libraries (see dependencies).

It integrates nicely with other CMake projects by defining (and exporting) a HighFive target.

### Design
- Simple C++-ish minimalist interface
- No other dependency than libhdf5
- Zero overhead
- Support C++14

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
- HDF5 or pHDF5, including headers
- boost >= 1.41 (recommended)
- eigen3 (optional)
- xtensor (optional)
- half (optional)

### Known flaws
- HighFive is not thread-safe. At best it has the same limitations as the HDF5 library. However, HighFive objects modify their members without protecting these writes. Users have reported that HighFive is not thread-safe even when using the threadsafe HDF5 library, e.g., https://github.com/BlueBrain/HighFive/discussions/675.
- Eigen support in core HighFive was broken until v3.0. See https://github.com/BlueBrain/HighFive/issues/532. H5Easy was not
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

**Note:** As of 2.8.0, one can use `highfive/highfive.hpp` to include
everything HighFive. Prior to 2.8.0 one would include `highfive/H5File.hpp`.

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
    - [Eigen::Matrix](http://eigen.tuxfamily.org) (optional),
    - [xt::xarray](https://github.com/QuantStack/xtensor) and [xt::xtensor](https://github.com/QuantStack/xtensor)
      (optional).
    - [cv::Mat_](https://docs.opencv.org/master/df/dfc/classcv_1_1Mat__.html)
      (optional).

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

whereby the `int` type of this example can be replaced by any of the above
types. See [easy_load_dump.cpp](src/examples/easy_load_dump.cpp) for more
details.

**Note:** Classes such as `H5Easy::File` are just short for the regular
`HighFive` classes (in this case `HighFive::File`). They can thus be used
interchangeably.


## CMake integration
There's two common paths of integrating HighFive into a CMake based project.
The first is to "vendor" HighFive, the second is to install HighFive as a
normal C++ library. Since HighFive makes choices about how to integrate HDF5,
sometimes following the third Bailout Approach is needed.

Regular HDF5 CMake variables can be used. Interesting variables include:

* `HDF5_USE_STATIC_LIBRARIES` to link statically against the HDF5 library.
* `HDF5_PREFER_PARALLEL` to prefer pHDF5.
* `HDF5_IS_PARALLEL` to check if HDF5 is parallel.

Please consult `tests/cmake_integration` for examples of how to write libraries
or applications using HighFive.

### Vendoring HighFive

In this approach the HighFive sources are included in a subdirectory of the
project (typically as a git submodule), for example in `third_party/HighFive`.

The projects `CMakeLists.txt` add the following lines
```cmake
add_subdirectory(third_party/HighFive)
target_link_libraries(foo HighFive)
```

**Note:** `add_subdirectory(third_party/HighFive)` will search and "link" HDF5
but wont search or link any optional dependencies such as Boost.

### Regular Installation of HighFive

Alternatively, HighFive can be install and "found" like regular software.

The project's `CMakeLists.txt` should add the following:
```cmake
find_package(HighFive REQUIRED)
target_link_libraries(foo HighFive)
```

**Note:** `find_package(HighFive)` will search for HDF5. "Linking" to
`HighFive` includes linking with HDF5. The two commands will not search for or
"link" to optional dependencies such as Boost.

### Bailout Approach

To prevent HighFive from searching or "linking" to HDF5 the project's
`CMakeLists.txt` should contain the following:

```cmake
# Prevent HighFive CMake code from searching for HDF5:
set(HIGHFIVE_FIND_HDF5 Off)

# Then "find" HighFive as usual:
find_package(HighFive REQUIRED)
# alternatively, when vendoring:
# add_subdirectory(third_party/HighFive)

# Finally, use the target `HighFive::Include` which
# doesn't add a dependency on HDF5.
target_link_libraries(foo HighFive::Include)

# Proceed to find and link HDF5 as required.
```

### Optional Dependencies

HighFive does not attempt to find or "link" to any optional dependencies, such
as Boost, Eigen, etc. Any project using HighFive with any of the optional
dependencies must include the respective header:
```
#include <highfive/boost.hpp>
#include <highfive/eigen.hpp>
```
and add the required CMake code to find and link against the dependencies. For
Boost the required lines might be
```
find_package(Boost REQUIRED)
target_link_libraries(foo PUBLIC Boost::headers)
```

# Questions?

Do you have questions on how to use HighFive? Would you like to share an interesting example or
discuss HighFive features? Head over to the [Discussions](https://github.com/BlueBrain/HighFive/discussions)
forum and join the community.

For bugs and issues please use [Issues](https://github.com/BlueBrain/HighFive/issues).

# Funding & Acknowledgment
 
The development of this software was supported by funding to the Blue Brain Project, a research center of the École polytechnique fédérale de Lausanne (EPFL), from the Swiss government's ETH Board of the Swiss Federal Institutes of Technology.

HighFive releases are uploaded to Zenodo. If you wish to cite HighFive in a
scientific publication you can use the DOIs for the
[Zenodo records](https://zenodo.org/doi/10.5281/zenodo.10679422).
 
Copyright © 2015-2022 Blue Brain Project/EPFL


### License

Boost Software License 1.0
