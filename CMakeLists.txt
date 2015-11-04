project(HighFive)
cmake_minimum_required(VERSION 2.8)

enable_testing()

option(UNIT_TESTS "enable unit tests" TRUE)

find_package(HDF5 REQUIRED)

add_subdirectory(src/examples)


add_subdirectory(tests/unit)
