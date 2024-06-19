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
double x[n][m];
dset.read_raw((double*) x);
```

### Accidental Raw Read
We consider the example above to be accidentally using a raw read, when it
could be performing a regular read. We suggest to not change the above, i.e.
```
double x[n][m];
dset.read(x);
```
continues to be correct in `v3` and can check that the dimensions match. The
inspector recognizes `double[n][m]` as a contiguous array of doubles.
Therefore, it'll use the shallow-copy buffer and avoid the any additional
allocations or copies.

### Intentional Raw Read
When genuinely performing a "raw read", one must replace `read` with
`read_raw`. For example:

```
double* x = malloc(n*m * sizeof(double));
dset.read_raw(x);
```
is correct in `v3`.

## Change for `T**`, `T***`, etc.
*The immediately preceding section is likely relevant.*

In `v2` raw pointers could be used to indicate dimensionality. For example:
```
double* x = malloc(n*m * sizeof(double));
auto dset = file.createDataSet("foo", DataSpace({n, m}), ...);

dset.write((double**) x);
dset.read((double**) x);
```
was valid and would write the flat array `x` into the two-dimensional dataset
`"foo"`. This must be modernized as follows:
```
double* x = malloc(n*m * sizeof(double));
auto dset = file.createDataSet("foo", DataSpace({n, m}), ...);

dset.write_raw(x);
dset.read_raw(x);
```

In `v3` the type `T**` will refer a pointer to a pointer (as usual). The
following:
```
size_t n = 2, m = 3;
double** x = malloc(n * sizeof(double*));
for(size_t i = 0; i < n; ++i) {
  x[i] = malloc(m * sizeof(double));
}

auto dset = file.createDataSet("foo", DataSpace({n, m}), ...);
dset.write(x);
dset.read(x);
```
is correct in `v3` but would probably segfault in `v2`.


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
# Option 1: HighFive is installed as a (system-wide) regular library:
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

## Removal of broadcasting.
HighFive v2 had a feature that a dataset (or attribute) of shape `[n, 1]` could
be read into a one-dimensional array automatically.

The feature is prone to accidentally not failing. Consider an array that shape
`[n, m]` and in general both `n, m > 0`. Hence, one should always be reading
into a two-dimensional array, even if `n == 1` or `m == 1`. However, due to
broadcasting, if one of the dimensions (accidentally) happens to be one, then
the checks wont fails. This isn't a bug, however, it can hide a bug. For
example if the test happen to use `[n, 1]` datasets and a one-dimensional
array.

Broadcasting in HighFive was different from broadcasting in NumPy. For reading
into one-dimensional data HighFive supports stripping all dimensions that are
not `1`. When extending the feature to multi-dimensional arrays it gets tricky.
We can't strip from both the front and back. If we allow stripping from both
ends, arrays such as `[1, n, m]` read into `[n, m]` if `m > 1` but into `[1,
n]` (instead of `[n, 1]`) if (coincidentally) `m == 1`. For HighFive because
avoiding being forced to read `[n, 1]` into `std::vector<std::vector<T>>` is
more important than `[1, n]`.  Flattening the former requires copying
everything while the latter can be made flat by just accessing the first value.
Therefore, HighFive had a preference to strip from the right, while NumPy adds
`1`s to the front/left of the shape.

In `v3` we've removed broadcasting. Instead users must use one of the two
alternatives: squeezing and reshaping. The examples show will use datasets and
reading, but it works the same for attributes and writing.

### Squeezing
Often we know that the `k`th dimension is `1`, e.g. a column is `[n, 1]` and a
row is `[1, m]`. In this case it's convenient to state, remove dimension `k`.
The syntax to simultaneously remove the dimensions `{0, 2}` is:

```
dset.squeezeMemSpace({0, 2}).read(array);
```
Which will read a dataset with dimensions `[1, n, 1]` into an array of shape
`[n]`.

### Reshape
Sometimes it's easier to state what the new shape must be. For this we have the
syntax:
```
dset.reshapeMemSpace(dims).read(array);
```
To declare that `array` should have dimensions `dims` even if
`dset.getDimensions()` is something different.

Example:
```
dset.reshapeMemSpace({dset.getElementCount()}).read(array);
```
to read into a one-dimensional array.

### Scalars
There's a safe case that seems needlessly strict to enforce: if the dataset is
a multi-dimensional array with one element one should be able to read into
(write from) a scalar.

The reverse, i.e. reading a scalar value in the HDF5 file into a
multi-dimensional array isn't supported, because if we want to support array
with runtime-defined rank, we can't deduce the correct shape, e.g. `[1]` vs.
`[1, 1, 1]`, when read into an array.

## Change to `File::Truncate` and friends.
In `v2`, `File::{ReadOnly,Truncate,...}` was an anonymous member enum of
`File`. Effectively it's type was the same as an `int`.

To improve type-safety, we converted it into an `enum class` called
`File::AccessMode`. In order to reduce the migration effort, we retained the
ability to write: `File::ReadOnly`.

Functions that accept a file access mode should be modernized as follows:
```
// old
HighFive::File open(std::string name, int mode) {
  return HighFive::File(name, mode);
}

// new
HighFive::File open(std::string name, HighFive::File::AccessMode mode) {
  return HighFive::File(name, mode);
}
```

Note: There's a caveat, the short-hand notation `File::ReadOnly` doesn't have
an address. Meaning one can't take it's address or const-references of it
(results in a linker error about missing symbol `File::ReadOnly`). Use
`File::AccessMode::ReadOnly` instead.

## Removal of `Object*Props`.
To our knowledge these could not be used meaningfully. Please create an issue
if you relied on these.
