# Changes
## Version 3.0.0-beta2 - 2024-12-04
### New Features
    - Support `boost::span`. (#1025)

### Bug Fix
    - Fix for not-quite null-terminated, fixed-length strings. (#1056)
    - Guard target creation in `*Config.cmake`. (#1053)

## Version 3.0.0-beta1 - 2024-07-16
This version is a major one and is breaking some usage compare to v2.
Read the migration guide from the documentation: https://bluebrain.github.io/HighFive/md__2home_2runner_2work_2_high_five_2_high_five_2doc_2migration__guide.html

The minimum version for C++ has been moved to `C++14`.

### Removed
    - Removed `read(T*, ...)`, use explicit `read_raw(T*, ...)` for `Slice` or `Attribute`. (#928)
    - Removed `FixedLenStringArray`, use any container with strings instead. (#932)
    - Removed `FileDriver` and `MPIOFileDriver`, use file access properties instead. (#949)
    - Removed default constructor for `Group` and `DataSet`. (#947, #948)
    - Broadcasting have been removed. Use `squeeze` and `reshape` feature instead. (#992)
    - `ObjectCreateProps` and `ObjectAccessProps` those don't map well to HighFive and are unused. (#1002)

### New Features
    - Added support for `std::span`. (#987)
    - Added `squeezeMemSpace` and `reshapeMemSpace` for `Attribute` and `Slice` to reshape the memory space. (#991)
    - Added `ProductSet` to select a Cartesian products of (generalized) slices. (#842)

### Improvements
    - Optimized chained hyperslab selection. (#1031)
    - Type `T[N]` or `T[N][M]` will work better. (#929)
    - `DataspaceType` is now an enum class for `dataspace_scalar` or `dataspace_null`. (#900)
    - `File::AccessMode` is now an enum class. (#1020)

## Version 2.9.0 - 2024-01-25
### New Features
    - Add named ctors for scalar and null dataspaces. (#899)
    - Named ctor for empty property lists. (#904)

### Improvements
    - Enable running tests in parallel. (#849)
    - Wrap all used HDF5 function calls and always check status codes. (#863)
    - Utilities for writing tests in a container independent manner. (#871)
    - Improve test rigour.

### Bug Fix
    - Log messages were slightly misformatted. (#854)
    - Fix bug in `enforce_ascii_hack`. (#856)
    - Fix `create_datatype<bool>()`. (#869)
    - Guard functionality introduced in 1.10.0. (#905)
    - `inspector` guard for empty containers. (#913)
    - Avoid non-collective behaviour. (#912)


## Version 2.8.0 - 2023-11-02
### Important Change
    - `Eigen::Matrix` is (by default) stored with column-major index ordering. Under
      certain conditions `Eigen::Matrix` was written and read as row-major.
      Due to code duplication H5Easy isn't affected by this bug. Starting
      `2.8.0` HighFive will now throw an exception whenever prior versions would
      have read with incorrect assumptions about the index ordering. (#731)

### New Features
    - Improve reading and writing `std::string` as fixed and variable length HDF5 strings (#744).
    - Implement creation of hard links (#765). Thanks to @Quark-X10.
    - Get the size of file and amound of tracked unused space (#764). Thanks to @Quark-X10.
    - `class DataType` has a new ctor to open a commited `DataType` (#796). Thanks to @Quark-X10.
    - Allow user-specified `mem_space` for hyperslabs. (#740)
    - New properties: `AttributePhaseChange`. (#785)
    - New options to link against HDF5 statically (#823). Thanks @HunterBelanger.
    - Add support for `std::complex<integral_type>` valid with C++23 (#828). Thanks @unbtorsten.
    - Add a top-level header to include all compononents (#818).

### Improvements
    - Add concept checks to `Property` if C++20 for better errors (#811). Thanks @antonysigma.
    - Add parallel HDF5 test in CI (#760).
    - Simplify github workflow (#761).
    - Move inspectors in their own file to be able to better implements strings (#759).

### Bug Fix
    - Fix vector constructor ambiguity in H5DataType.hpp (#775). Thanks to @hn-sl.
    - `getElementCount()` fixed. (#787)
    - Remove leak when calling dtor of `CompoundType`. (#798)

## Version 2.7.1 - 2023-04-04
### Bug Fix
    - Revert removing `#include "H5FileDriver.hpp"` from `H5File.hpp` (#711).
    - Change relative import to "../H5Utility.hpp" (#726).
    - Fix nameclash with macros on Windows (#717 #722 #723).
    - Add workaround for MSVC bug (#728).
    - Don't downgrade the requested C++ standard (#729).

## Version 2.7.0 - 2023-03-31
### New Features
    - Properties can now be read (#684).
    - Adding a property for LinkCreationOrder (#683).
    - Adding a logging infrastructure (#690).
    - Support of bool in the way of h5py (#654).
    - Support `std::bool` in C++17 mode (#698).

### Improvements
    - Catch2 move to v3 (#655).

### Bug Fix
    - To avoid build failure in certain circumstances, user can not set `Boost_NO_BOOST_CMAKE` (#687).
    - Fix leak when reading variable length strings (#660).
    - Use `H5free_memory` instead of `free` in error handler (#665). Thanks to Moritz Koenemann.
    - Fix a bug with old GCC due to templated friend classes (#688).
    - Fix regression in broadcasting support (#697).
    - Fix bug related to zero-length datasets (#702).

## Version 2.6.2 - 2022-11-10
### Bug Fix
    - Allow CMake to use Config mode to find HDF5.

## Version 2.6.1 - 2022-11-08
### Bug Fix
    - Version bump in `CMakeLists.txt`.

## Version 2.6.0 - 2022-11-08
### New Features
    - Enable page buffered reading (#639).

### Improvements
    - Warn when detecting lossy reads or write of floating point data (#636).

## Version 2.5.1 - 2022-11-07
### Bug Fix
    - Fix missing `inline` for collective metadata properties.

## Version 2.5.0 - 2022-11-03
### New Features
    - Enable collective MPI IO using the Data Transfer Property (#623). Thanks to Rob Latham.
    - Add a support for half-precision (16-bit) floating-point based on the Half library (http://half.sourceforge.net) (#587). Thanks to Sergio Botelh.
    - Enable choosing the allocation time of datasets (#627).
    - Add possibility to get and set file space strategy. For page allocated files wrap the API to set/retrieve the page size (#618).
    - Add API for getting Access and Create property lists of HighFive objects (#629).
    - Let users configure metadata reads and writes at file level (#624). Thanks to Rob Latham.

### Improvements
    - MPIOFileDriver is now deprecated. Use FileAccessProps (#622).
    - Support of block argument in API (#584).
    - Serialization of types is now automagic and so recursive (#586).
    - Add an argument to specific File Create Properties in File class construtor (#626).

### Bug Fixes
    - Padding of Compound Types (#581).
    - Compilation with Visual Studio with C++17 or later (#578). Thanks to Mark Bicknell.
    - Avoid leaking when printing stack for error (#583).

## Version 2.4.1 - 2022-05-11
### New Features
    - Support `std::complex`. Thanks to Philipp.

### Improvements
    - Improve EnumType/CompoundType
    - Revert quirky behaviour of `select(const HyperSlab&)`.
    - All `get_name` functions takes `size_t` and not `hsize_t`.
    - Remove nix recipes.

### Bug Fixes
    - Computation of padding.
    - Related to `0` being an invalid hid but not equal to `H5I_INVALID_HID`.

## Version 2.4.0 - 2022-04-05
### New Features
    - Construct a compound type from an already existing hid (#469). Thanks to Maximilian Nöthe.
    - Add support for long double (#494)
    - Add support for H5Pset_libver_bounds and H5Pset_meta_block_size support (#500)
    - New interface to select complex hyperslabs, irregular hyperslabs are limited to/from 1D array (#538 and #545)
### Improvements
    - Use inline where it is needed, otherwise some code can lead to "multiple definition" (#516). Thanks to Chris Byrohl.
    - Use Catch2 instead of boost for tests, reduces dependencies (#521)
    - CI reworked to test external libraries more thoroughly (boost, eigen, xtensor) (#536)
### Bug Fixes
    - Better support of const types (#460). Thanks to Philip Deegan.
    - Vector of size zero was previously lead to UB (#502). Thanks to Haoran Ni.
    - Use H5T_NATIVE_SCHAR instead of H5T_NATIVE_CHAR for "signed char" (#518)

## Version 2.3.1 - 2021-08-04
### Improvements
    - Clean cmake files from old code (#465)
    - Adding path to type warning message (#471)
    - Adding compound types example, w dataset and attr (#467)

### Bug Fixes
    - Resolve an issue where padding of nested compound types were being calculated incorrectly (#461) (#468)
    - GHA: drop previous runs (#462)

## Version 2.3 - 2021-05-07
### New Features:
    - Add SZIP support (#435)
    - Add option *parents* to createDataSet (#425)
    - Implementing getting the filename dynamically (#424)
    - Ability to create soft and external links (#421)
    - Generalizing getPath and adding getFile as PathTraits (#417)

### Improvements:
    - Unified reading/writing attributes and datasets (#450)
    - Old compilers have been removed from docker image (#430)
    - Cleaning up and improving property lists (#429)
    - An example using hdf5 references (#396) (#397)
    - Add all property lists alias for completeness (#427)
    - Add property CreateIntermediateGroup (#423)
    - Add code coverage through codecov.io (#420)
    - Introducing GitHub Actions CI (#416)
    - Create issue and PR templates (#412)
    - Initialize SilenceHDF5 to true in _exist (#411)
    - Generalizing xtensor API (#407)
    - Minor doc updates (#409)
    - Fixing minor error in GH Action (#408)
    - Uploading docs to gh-pages using GitHub Actions (#403)
    - Various minor documentation updates (#405)
    - optional documentation building in cmake (#377)
    - From can be automatic now (#384)
    - get_dim_vector in inspector (#383)
    - Put type_of_array in inspector (#382)
    - Move array_dims in the future manipulator (#381)
    - Unify interface of H5Attribute with H5Slice_traits (#378)
    - Use std::move in NRVO depending of version of GCC (#375)
    - Fixed typo '-DD' to '-D' in 'Dependencies'. (#371)
    - Changing date format (#364)

### Bug fixes:
    - Fix use before initialization (#414)
    - Adding CMake include guard (#389)

## Version 2.2.2 - 2020-07-30
### New Features:
    - [H5Easy] Adding OpenCV support (#343)
    - [H5Easy] Enabling compression & Adding attributes (#337)
    - Adding missing function to H5Attribute (#337) 
    - Add methods to retrieve Node paths or Dataset names and rename objects (#346)
    - Add a file with the current version number of HighFive (#349)

### Improvements
    - [H5Easy] Updating error message dump (#335)
    - [H5Easy] Switching implementation to partial specialization based on static dispatch (#327)
    - Simplifying imports, new policy (#324)

## Version 2.2.1 - 2020-04-28
### Improvements
    - Add a mechanism to not include target HighFive several times (#336)
    - Fix SilenceHDF5 initialization for NodeTraits (#333)

## Version 2.2 - 2020-03-23
### New Features:
    - Compound Types: API to register and read/write structs (#78). Thanks to Richard Shaw.
    - Fixed-length strings. API via char[] and `FixedLenStringArray`(#277)
    - Enum data types (#297)
    - Datasets of HDF5 References. Support to dereference groups and datasets (#306)
    - Objects (hard/soft link) can now be deleted with `unlink` (#284). Thanks to Tom Vander Aa.
    - Attributes can be deleted with `deleteAttribute` (#239)

### Improvements:
    - `Attribute`s (metadata) now support additional types (#298)
    - H5Easy: Reworked for compatibility with `Eigen::ref` and `Eigen::Map` (#291, #293)
    - Hdf5 1.12 compatibility: working `Object::getInfo` and marking getAddress deprecated (#311)
    - Strict compatibility with CMake 3.1 and C++11 (#304)
    - CMake: Dependencies may be re-detected on FindPackage, fixed export targets and added integration tests (#255, #304, #312, #317)
    - Support for array of `Eigen::Matrix` (#258)
    - Selection: `ElementSet` working for N-dimensions (#247)

### Bug Fixes:
    - Shortcut syntax with c arrays (#273)
    - Compatibility with in MSVC (Exception messages #263 and avoid throwing in `exist` check #308)

## Version 2.1 - 2019-10-30
### New Features:
    - Inspection: API to get the type of links/objects and datasets data-types (#221)
    - H5Easy: API for simple import/export to Eigen and xtensor (#141)
    - Support for chunk and deflate configuration at dataset creation/open (#125). Added generic RawPropertyLists. (#157)
    - Recursive `createGroup` and `exist` (#152)
    - Shortcut syntax: ability to create a filled dataset in a single line (#130)
    - DataSet now accepts `std::complex` and `std::array`'s (#128, #129)

### Improvements:
    - Improved compat with MSVC and ICC compilers
    - CMake build system: modernized, create exported targets, better messages, etc.
    - Building and publishing documentation: https://bluebrain.github.io/HighFive/
    - Several other. See #231

### Bug Fixes:
    - Fixed header dependencies. They are now all include-able (#225)
    - Fixed read/write of N-Dimensional data as nested vectors (#191)
    - Fixed data broadcasting for reading (#136)

## Version 2.0 - 2018-07-19
    - First version with C++11 enforcement
    - Support for property list
    - Support for Chunking
    - Support for Compression / Deflate
    - Fix: missing move constructor for properties
    - Fix: typo in MPI IO driver
    - Fix: several typo fixes
    - Fix: Add missing include

## Version 1.5 - 2018-01-06
    - SliceTraits::read split in two overloads, the first one for plain C arrays
      and the second one for other types.
    - Add support for complex number
    - Add exist() method to the API
    - Will be last release before 2.0 and enforcement of C++11

## Version 1.4 - 2017-08-25
	- Support id selection for the `select` function
	- Suport STL containers of const elements
	- Support scalar values and strings management
	- Fix attribute assignment issue #40
    - Fix Object assignment operator missing unref (possible memory leak )
    - Introduce SilenceHDF5 for HDF5 error report
    - Fix a unit test issue with SilenceHDF5

## Version 1.3 - 2017-06-21
    - Minor fixes

## Version 1.2 - 2017-04-03
	- Add Attribute support for Dataset
	- Extend testing of Attribute support
	- Fix issue related to multiple definitions in default driver
	- Add more examples about attribute support

## Version 1.1 - 2017-03-23
    - Add support and examples for Parallel HDF5
    - Initial implementation for H5 Properties
    - Support for Attributes
    - Improve documentation
    - Add example for boost.Ublas matrix support

## Version 1.0 - Init
	- Initial release
