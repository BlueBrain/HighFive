project(HighFive)
cmake_minimum_required(VERSION 2.8)

enable_testing()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
      ${PROJECT_SOURCE_DIR}/CMake/portability
      ${PROJECT_SOURCE_DIR}/CMake/packages
      ${PROJECT_SOURCE_DIR}/CMake/config)

include(DefineInstallationPaths)
include(ReleaseDebugAutoFlags)

option(UNIT_TESTS "enable unit tests" TRUE)

find_package(HDF5 REQUIRED)

add_subdirectory(src/examples)


add_subdirectory(tests/unit)
