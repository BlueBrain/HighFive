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
option(USE_BOOST "enable Boost Support" TRUE)

find_package(HDF5 REQUIRED)

if(USE_BOOST)
add_definitions( -DH5_USE_BOOST )
endif()

add_subdirectory(src/examples)


add_subdirectory(tests/unit)
