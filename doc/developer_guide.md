# Developer Guide
First clone the repository and remember the `--recursive`:
```bash
git clone --recursive git@github.com:BlueBrain/HighFive.git
```
The instructions to recover if you forgot are:
```bash
git submodule update --init --recursive
```

One remark on submodules: each HighFive commit expects that the submodules are
at a particular commit. The catch is that performing `git checkout` will not
update the submodules automatically. Hence, sometimes a `git submodule update
--recursive` might be needed to checkout the expected version of the
submodules.

## Compiling and Running the Tests
The instructions for compiling with examples and unit-tests are:

```bash
cmake -B build -DCMAKE_BUILD_TYPE={Debug,Release} .
cmake --build build --parallel
ctest --test-dir build
```

You might want to add:
* `-DHIGHFIVE_TEST_BOOST=On` or other optional dependencies on,
* `-DHIGHFIVE_MAX_ERRORS=3` to only show the first three errors.

Generic CMake reminders:
* `-DCMAKE_INSTALL_PREFIX` defines where HighFive will be installed,
* `-DCMAKE_PREFIX_PATH` defines where `*Config.cmake` files are found.

## Contributing
There's numerous HDF5 features that haven't been wrapped yet. HighFive is a
collaborative effort to slowly cover ever larger parts of the HDF5 library.
The process of contributing is to fork the repository and then create a PR.
Please ensure that any new API is appropriately documented and covered with
tests.

### Code formatting
The project is formatted using clang-format version 12.0.1 and CI will complain
if a commit isn't formatted accordingly. The `.clang-format` is at the root of
the git repository. Conveniently, `clang-format` is available via `pip`.

Formatting the entire code base can be done with:
```bash
    bin/format.sh
```
which will install the required version of clang-format in a venv called
`.clang-format-venv`.

To format only the changed files `git-clang-format` can be used.

## Releasing HighFive
Before releasing a new version perform the following:

* Update `CHANGELOG.md` and `AUTHORS.txt` as required.
* Update `CMakeLists.txt` and `include/highfive/H5Version.hpp`.
* Follow semantic versioning when deciding the next version number.
* Check that
  [HighFive-testing](https://github.com/BlueBrain/HighFive-testing/actions) ran
  recently.

At this point there should be a commit on master which will be the release
candidate. Don't tag it yet.

Next step is to update the [HighFive/spack](https://github.com/BlueBrain/spack)
recipe such that the proposed version points to the release candidate using the
SHA of that commit. The recipe will look something like this:

```python
    # ...

    version("2.8.0", commit="094400f22145bcdcd2726ce72888d9d1c21e7068")
    version("2.7.1", sha256="25b4c51a94d1e670dc93b9b73f51e79b65d8ff49bcd6e5d5582d5ecd2789a249")
    version("2.7.0", sha256="8e05672ddf81a59ce014b1d065bd9a8c5034dbd91a5c2578e805ef880afa5907")
    # ...
```

Push the changes to the BlueBrain spack repository. This will trigger building
all BBP dependencies of HighFive, i.e. another integration test. Don't actually
merge this commit yet.

Now that we know that the integration test ran, and all BBP software can be
built with the proposed version of HighFive, we can proceed and create the
release. Once this is done perform a final round of updates:

* Download the archive (`*.tar.gz`) and compute its SHA256.
* Update BlueBrain Spack recipe to use the archive and not the Git commit.
* Update the upstream Spack recipe.
* A Zenodo record should be generated automatically, check if it's sensible.

## Writing Tests
### Generate Multi-Dimensional Test Data
Input array of any dimension and type can be generated using the template class
`DataGenerator`. For example:
```
auto dims = std::vector<size_t>{4, 2};
auto values = testing::DataGenerator<std::vector<std::array<double, 2>>::create(dims);
```
Generates an `std::vector<std::array<double, 2>>` initialized with suitable
values.

If "suitable" isn't specific enough, one can specify a callback:
```
auto callback = [](const std::vector<size_t>& indices) {
    return 42.0;
}

auto values = testing::DataGenerator<std::vector<double>>::create(dims, callback);
```

The `dims` can be generated via `testing::DataGenerator::default_dims` or by
using `testing::DataGenerator::sanitize_dims`. Remember, that certain
containers are fixed size and that we often compute the number of elements by
multiplying the dims.

### Generate Scalar Test Data
To generate a single "suitable" element use template class `DefaultValues`, e.g.
```
auto default_values = testing::DefaultValues<double>();
auto x = testing::DefaultValues<double>(indices);
```

### Accessing Elements
To access a particular element from an unknown container use the following trait:
```
using trait = testing::ContainerTraits<std::vector<std::array<int, 2>>;
// auto x = values[1][0];
auto x = trait::get(values, {1, 0});

// values[1][0] = 42.0;
trait::set(values, {1, 0}, 42.0);
```

### Utilities For Multi-Dimensional Arrays
Use `testing::DataGenerator::allocate` to allocate an array (without filling
it) and `testing::copy` to copy an array from one type to another. There's
`testing::ravel`, `testing::unravel` and `testing::flat_size` to compute the
position in a flat array from a multi-dimensional index, the reverse and the
number of element in the multi-dimensional array.

### Deduplicating DataSet and Attribute
Due to how HighFive is written testing `DataSet` and `Attribute` often requires
duplicating the entire test code because somewhere a `createDataSet` must be
replaced with `createAttribute`. Use `testing::AttributeCreateTraits` and
`testing::DataSetCreateTraits`. For example,
```
template<class CreateTraits>
void check_write(...) {
    // Same as one of:
    //   file.createDataSet(name, values);
    //   file.createAttribute(name, values);
    CreateTraits::create(file, name, values);
}
```

### Test Organization
#### Multi-Dimensional Arrays
All tests for reading/writing whole multi-dimensional arrays to datasets or
attributes belong in `tests/unit/test_all_types.cpp`. This
includes write/read cycles; checking all the generic edges cases, e.g. empty
arrays and mismatching sizes; and checking non-reallocation.

Read/Write cycles are implemented in two distinct checks. One for writing and
another for reading. When checking writing we read with a "trusted"
multi-dimensional array (a nested `std::vector`), and vice-versa when checking
reading. This matters because certain bugs, like writing a column major array
as if it were row-major can't be caught if one reads it back into a
column-major array.

Remember, `std::vector<bool>` is very different from all other `std::vector`s.

Every container `template<class T> C;` should at least be checked with all of
the following `T`s that are supported by the container: `bool`, `double`,
`std::string`, `std::vector`, `std::array`. The reason is `bool` and
`std::string` are special, `double` is just a POD, `std::vector` requires
dynamic memory allocation and `std::array` is statically allocated.

Similarly, each container should be put inside an `std::vector` and an
`std::array`.

#### Scalar Data Set
Write-read cycles for scalar values should be implemented in
`tests/unit/tests_high_five_scalar.cpp`.

#### Data Types
Unit-tests related to checking that `DataType` API, go in
`tests/unit/tests_high_data_type.cpp`.

#### Empty Arrays
Check related to empty arrays to in `tests/unit/test_empty_arrays.cpp`.

#### Selections
Anything selection related goes in `tests/unit/test_high_five_selection.cpp`.
This includes things like `ElementSet` and `HyperSlab`.

#### Strings
Regular write-read cycles for strings are performed along with the other types,
see above. This should cover compatibility of `std::string` with all
containers. However, additional testing is required, e.g. character set,
padding, fixed vs. variable length. These all go in
`tests/unit/test_string.cpp`.

#### Specific Tests For Optional Containers
If containers, e.g. `Eigen::Matrix` require special checks those go in files
called `tests/unit/test_high_five_*.cpp` where `*` is `eigen` for Eigen.

#### Memory Layout Assumptions
In HighFive we make assumptions about the memory layout of certain types. For
example, we assume that
```
auto array = std::vector<std::array<double, 2>>(n);
doube * ptr = (double*) array.data();
```
is a sensible thing to do. We assume similar about `bool` and
`details::Boolean`. These types of tests go into
`tests/unit/tests_high_five_memory_layout.cpp`.

#### H5Easy
Anything `H5Easy` related goes in files with the appropriate name.

#### Everything Else
What's left goes in `tests/unit/test_high_five_base.cpp`. This covers opening
files, groups, dataset or attributes; checking certain pathological edge cases;
etc.
