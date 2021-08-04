cmake_minimum_required(VERSION 3.1)
if(${CMAKE_VERSION} VERSION_LESS 3.13)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
  cmake_policy(VERSION 3.13)
endif()

project(HighFive VERSION 2.3.1)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/highfive/H5Version.hpp.in
               ${CMAKE_CURRENT_BINARY_DIR}/include/highfive/H5Version.hpp)
# INCLUDES
list(APPEND CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/CMake
  ${PROJECT_SOURCE_DIR}/CMake/portability
  ${PROJECT_SOURCE_DIR}/CMake/config)

include(CompilerFlagsHelpers)
include(ReleaseDebugAutoFlags)

# OPTIONS
# Compat within Highfive 2.x series
set(USE_BOOST ON CACHE BOOL "Enable Boost Support")
set(USE_EIGEN OFF CACHE BOOL "Enable Eigen testing")
set(USE_XTENSOR OFF CACHE BOOL "Enable xtensor testing")
set(USE_OPENCV OFF CACHE BOOL "Enable OpenCV testing")
mark_as_advanced(USE_BOOST USE_EIGEN USE_XTENSOR)

option(HIGHFIVE_USE_BOOST "Enable Boost Support" ${USE_BOOST})
option(HIGHFIVE_USE_EIGEN "Enable Eigen testing" ${USE_EIGEN})
option(HIGHFIVE_USE_XTENSOR "Enable xtensor testing" ${USE_XTENSOR})
option(HIGHFIVE_USE_OPENCV "Enable OpenCV testing" ${USE_OPENCV})
option(HIGHFIVE_UNIT_TESTS "Enable unit tests" ON)
option(HIGHFIVE_EXAMPLES "Compile examples" ON)
option(HIGHFIVE_PARALLEL_HDF5 "Enable Parallel HDF5 support" OFF)
option(HIGHFIVE_BUILD_DOCS "Enable documentation building" ON)

# In deployments we probably don't want/cant have dynamic dependencies
option(HIGHFIVE_USE_INSTALL_DEPS "End applications by default use detected dependencies here" OFF)
mark_as_advanced(HIGHFIVE_USE_INSTALL_DEPS)


# Check compiler cxx_std requirements
# -----------------------------------

if(CMAKE_CXX_STANDARD EQUAL 98)
    message(FATAL_ERROR "HighFive needs to be compiled with at least C++11")
endif()

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if(HIGHFIVE_USE_XTENSOR)
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()


# Search dependencies (hdf5, boost, eigen, xtensor, mpi) and build target highfive_deps
include(${PROJECT_SOURCE_DIR}/CMake/HighFiveTargetDeps.cmake)

# Set-up HighFive to be used in 3rd party project using exports. Create a HighFive target
include(${PROJECT_SOURCE_DIR}/CMake/HighFiveTargetExport.cmake)

# Installation of headers (HighFive is only interface)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION "include"
  PATTERN "*.in" EXCLUDE)

# Installation of configured headers
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/
  DESTINATION "include")


# Preparing local building (tests, examples)
# ------------------------------------------

# Disable test if Boost was expressly disabled, or if HighFive is a sub-project
if (NOT HIGHFIVE_USE_BOOST OR NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  if(HIGHFIVE_UNIT_TESTS)
    message(WARNING "Unit tests have been DISABLED. (HIGHFIVE_USE_BOOST: ${HIGHFIVE_USE_BOOST})")
  endif()
  set(HIGHFIVE_UNIT_TESTS FALSE)
endif()

if(HIGHFIVE_UNIT_TESTS)
  set(Boost_NO_BOOST_CMAKE TRUE)  # Consistency
  find_package(Boost COMPONENTS system serialization unit_test_framework)
  if (NOT Boost_FOUND)
    message(FATAL_ERROR "\
      Boost not found which is required for efficient multi-dimension and tests.\n\
      To disable support please use cmake .. -DHIGHFIVE_USE_BOOST=OFF.")
  endif()
endif()


if(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
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

if(HIGHFIVE_BUILD_DOCS)
  add_subdirectory(doc)
endif()
