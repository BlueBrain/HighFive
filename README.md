> [!WARNING]
> The Blue Brain Project concluded in December 2024, so the HighFive development is ceased under the BlueBrain GitHub organization.
>
> The development of HighFive will continue at:
>   https://github.com/highfive-devs/highfive

# HighFive - HDF5 header-only C++ Library

Documentation: https://bluebrain.github.io/HighFive/

## Brief

HighFive is a modern header-only C++14 friendly interface for libhdf5.

HighFive supports STL vector/string, Boost::UBLAS, Boost::Multi-array and Xtensor. It handles C++ from/to HDF5 with automatic type mapping.
HighFive does not require additional libraries (see dependencies).

It integrates nicely with other CMake projects by defining (and exporting) a HighFive target.

### Design
- Simple C++-ish minimalist interface
- Only hard dependency is libhdf5
- Zero/low overhead, when possible
- RAII for opening/closing files, groups, datasets, etc.
- Written in C++14

### Feature support
- create/read/write files, datasets, attributes, groups, dataspaces.
- automatic memory management / ref counting
- automatic conversion of `std::vector` and nested `std::vector` from/to any dataset with basic types
- automatic conversion of `std::string` to/from variable- or fixed-length string dataset
- selection() / slice support
- parallel Read/Write operations from several nodes with Parallel HDF5
- Advanced types: Compound, Enum, Arrays of Fixed-length strings, References
- half-precision (16-bit) floating-point datasets
- `std::byte` in C++17 mode (with `-DCMAKE_CXX_STANDARD=17` or higher)
- etc... (see [ChangeLog](./CHANGELOG.md))

### Dependencies
- HDF5 or pHDF5, including headers
- boost (optional)
- eigen3 (optional)
- xtensor (optional)
- half (optional)

The releases for versions 2.x.y and two prereleases of v3 can be found at:
* https://github.com/BlueBrain/HighFive/releases
* https://zenodo.org/doi/10.5281/zenodo.10679422

The state of HighFive immediately before preparing it for archival is:
* https://github.com/BlueBrain/HighFive/tree/v3.0.0-beta2

All future development and new releases can be found at:
* https://github.com/highfive-devs/highfive

## Example

```c++
using namespace HighFive;

File file("foo.h5", File::Truncate);

{
    std::vector<int> data(50, 1);
    file.createDataSet("grp/data", data);
}

{
    auto dataset = file.getDataSet("grp/data");

    // Read back, automatically allocating:
    auto data = dataset.read<std::vector<int>>();

    // Alternatively, if `data` has the correct
    // size, without reallocation:
    dataset.read(data);
}
```

# Funding & Acknowledgment

The development of this software was supported by funding to the Blue Brain Project, a research center of the École polytechnique fédérale de Lausanne (EPFL), from the Swiss government's ETH Board of the Swiss Federal Institutes of Technology.

HighFive releases are uploaded to Zenodo. If you wish to cite HighFive in a
scientific publication you can use the DOIs for the
[Zenodo records](https://zenodo.org/doi/10.5281/zenodo.10679422).

Copyright © 2015-2024 Blue Brain Project/EPFL


### License

Boost Software License 1.0
