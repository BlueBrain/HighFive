cmake_minimum_required(VERSION 3.0)
if(${CMAKE_VERSION} VERSION_LESS 3.13)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
  cmake_policy(VERSION 3.13)
endif()

project(HighFive VERSION 2.1)

list(APPEND CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/CMake
  ${PROJECT_SOURCE_DIR}/CMake/portability
  ${PROJECT_SOURCE_DIR}/CMake/config)

include(CompilerFlagsHelpers)
include(ReleaseDebugAutoFlags)
include(CheckCXXStandardSupport)
include(BlueGenePortability)

if(NOT COMPILER_SUPPORTS_CXX11)
  message(FATAL_ERROR "HighFive version >= 2.0 requires c++ standard >= c++11")
endif()

option(HIGHFIVE_UNIT_TESTS "Enable unit tests" TRUE)
option(USE_BOOST "Enable Boost Support" TRUE)
option(USE_EIGEN "Enable Eigen testing" FALSE)
option(USE_XTENSOR "Enable xtensor testing" FALSE)
option(HIGHFIVE_EXAMPLES "Compile examples" TRUE)
option(HIGHFIVE_PARALLEL_HDF5 "Enable Parallel HDF5 support" FALSE)

# Disable test if Boost will not be loaded, or if HighFive is a sub-project
if (NOT USE_BOOST OR NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(HIGHFIVE_UNIT_TESTS FALSE)
endif()

# Pre-requisites

if(HIGHFIVE_PARALLEL_HDF5)
  set(HDF5_PREFER_PARALLEL TRUE)
endif()
find_package(HDF5 REQUIRED)

set(Boost_NO_BOOST_CMAKE TRUE)
if(USE_BOOST)
  set(BOOST_REQ_COMPONENTS system serialization)
  if(HIGHFIVE_UNIT_TESTS)
    list(APPEND BOOST_REQ_COMPONENTS unit_test_framework)
  endif()
  find_package(Boost COMPONENTS ${BOOST_REQ_COMPONENTS})
  if (NOT Boost_FOUND)
    message(FATAL_ERROR "\
      Boost not found which is required for efficient multi-dimension and tests.\n\
      To disable support please use cmake .. -DUSE_BOOST=OFF.")
  endif()
  add_definitions(-DH5_USE_BOOST)
endif()

if(USE_EIGEN)
  find_package(Eigen3 NO_MODULE)
  if(NOT Eigen3_FOUND)
    find_package(PkgConfig)
    pkg_check_modules(EIGEN3 REQUIRED eigen3)
  endif()
  if(Eigen3_FOUND)
    add_definitions(-DH5_USE_EIGEN)
    include_directories(SYSTEM ${EIGEN3_INCLUDE_DIRS})
  endif()
endif()

if(USE_XTENSOR)
  if(NOT COMPILER_SUPPORTS_CXX14)
    message(SEND_ERROR "C++ compiler does not support standard c++14, required to support xtensor.")
  else()
    set(CMAKE_CXX_STANDARD 14)
    find_package(xtensor REQUIRED)
    include_directories(SYSTEM $<TARGET_PROPERTY:xtensor,INTERFACE_INCLUDE_DIRECTORIES>)
    add_definitions(-DH5_USE_XTENSOR)
  endif()
endif()

if(HIGHFIVE_PARALLEL_HDF5 OR HDF5_IS_PARALLEL)
  find_package(MPI REQUIRED)
  add_definitions("-DMPI_NO_CPPBIND")
endif()

# HighFive as a sub-project and using the the same filename
include(${PROJECT_SOURCE_DIR}/CMake/PackageConfig.cmake)

# Some compilers default to old C++. If not chosen, select C++ 14 or 11 as available
select_recent_cxx_standard_available()

if(CMAKE_CXX_COMPILER_IS_ICC)
  # ICC gets mad if we shorten "int"s
  add_definitions("-wd1682")
endif()

if(HIGHFIVE_EXAMPLES)
  add_subdirectory(src/examples)
endif()

if(HIGHFIVE_UNIT_TESTS)
  enable_testing()
  add_subdirectory(tests/unit)
endif()

add_subdirectory(doc)
