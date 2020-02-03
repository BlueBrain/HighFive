HighFive documentation {#mainpage}
======================

HighFive is a modern C++11 friendly interface for libhdf5.

HighFive supports STL vector/string, eigen3, xtensor, Boost::UBLAS and Boost::Multi-array.

It handles C++ from/to HDF5 automatic type mapping.

HighFive does not require an additional library and supports both HDF5 thread safety and Parallel HDF5 (contrary to the official hdf5 cpp).

HighFive has two interfaces: normal HighFive and H5Easy.

H5Easy is a high-level interface composed by two main functions: H5Easy::load and H5Easy::dump.

HighFive is a wrapper of HDF5 library which take care of ownership of your HDF5 objects.
