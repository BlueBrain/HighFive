# Migration Guide
A collection of tips for migrating away from deprecated features.

## Deprecation of `FixedLenStringArray`.
The issue with `FixedLenStringArray` is that it is unable to avoid copies.
Essentially, this class acts as a means to create a copy of the data in a
format suitable for writing fixed-length strings. Additionally, the class acts
as a tag for HighFive to overload on. The support of `std::string` in HighFive
has improved considerable. Since 2.8.0 we can write/read `std::string` to fixed
or variable length HDF5 strings.

Therefore, this class serves no purpose anymore. Any occurrence of it can be
replaced with an `std::vector<std::string>` (for example).

If desired one can silence warnings by replacing `FixedLenStringArray` with
`deprecated::FixedLenStringArray`.


## Deprecation of `read(T*, ...)`.
A "raw read" is when the user allocates sufficient bytes and provides HighFive
with the pointer to the first byte. "Regular reads" take a detour via the
inspector and might resize the container, etc.

The issue is that HighFive `v2` had the following two overloads:
```
template<class T>
DataSet::read(T& x, /* skipped */);

template<class T>
DataSet::read(T* x, /* skipped */);
```
and the analogous for `Attribute`.

The issue is that the second overload will also match things like `T**` and
`T[][]`. For example the following code used the removed overload:
```
double x[2][3];
dset.read(x);
```
which is fine because is a contiguous sequence of doubles. It's equivalent to
following `v3` code:
```
double x[2][3];
dset.read_raw((double*) x);
```

### Accidental Raw Read
We consider the example above to be accidentally using a raw read, when it
could be performing a regular read. We suggest to not change the above, i.e.
```
double x[2][3];
dset.read(x);
```
continues to be correct in `v3` and can check that the dimensions match. The
inspector recognizes `double[2][3]` as a contiguous array of doubles.
Therefore, it'll use the shallow-copy buffer and avoid the any additional
allocations or copies.

### Intentional Raw Read
When genuinely performing a "raw read", one must replace `read` with
`read_raw`. For example:

```
double* x = malloc(2*3 * sizeof(double));
dset.read_raw(x);
```
is correct in `v3`.

## Reworked CMake
In `v3` we completely rewrote the CMake code of HighFive. Since HighFive is a
header only library, it needs to perform two tasks:

1. Copy the sources during installation.
2. Export a target that sets `-I ${HIGHFIVE_DIR}` and links with HDF5.

We've removed all flags for optional dependencies, such as
`-DHIGHFIVE_USE_BOOST`. Instead user that want to read/write into/from
optionally supported containers, include a header with the corresponding name
and make sure to adjust their CMake code to link with the dependency.

The C++ code should have:
```
#include <highfive/boost.hpp>

// Code the reads or write `boost::multi_array`.
```
and the CMake code would have
```
add_executable(app)

# These lines might work, but depend on how exactly the user intends to use
# Boost. They are not specific to HighFive, but previously added automatically
# (and sometimes correctly) by HighFive.
find_package(Boost)
target_link_libraries(add PUBLIC boost::boost)

# For HighFive there's two options for adding `-I ${HIGHFIVE_DIR}` and the
# flags for HDF5.
#
# Option 1: HighFive is install (systemwide) as a regular library:
find_package(HighFive)
target_link_libraries(app PUBLIC HighFive::HighFive)

# Option 2: HighFive is vendored as part of the project:
add_subdirectory(third_party/HighFive)
target_link_libraries(app PUBLIC HighFive::HighFive)
```

There are extensive examples of project integration in `tests/cmake_integration`,
including how those projects in turn can be included in other projects. If these
examples don't help, please feel free to open an Issue.

## Type change `DataSpace::DataSpaceType`.
We've converted the `enum` `DataSpace::DataSpaceType` to an `enum class`. We've
added static `constexpr` members `dataspace_null` and `dataspace_scalar` to
`DataSpace`. This minimizes the risk of breaking user code.

Note that objects of type `DataSpace::DataSpaceType` will no longer silently
convert to an integer. Including the two constants
`DataSpace::dataspace_{scalar,null}`.

## Deprecation `FileDriver` and `MPIOFileDriver`.
These have been deprecated to stick more closely with familiar HDF5 concepts.
The `FileDriver` is synonymous to `FileAccessProps`; and `MPIOFileDriver` is
the same as:
```
auto fapl = FileAccessProps{};
fapl.add(MPIOFileAccess(mpi_comm, mpi_info));
```

We felt that the savings in typing effort weren't worth introducing the concept
of a "file driver". Removing the concept hopefully makes it easier to add a
better abstraction for the handling of the property lists, when we discover
such an abstraction.
