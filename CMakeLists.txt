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

# Compat within Highfive 2.x series
set(USE_BOOST ON CACHE BOOL "Enable Boost Support")
set(USE_EIGEN OFF CACHE BOOL "Enable Eigen testing")
set(USE_XTENSOR OFF CACHE BOOL "Enable xtensor testing")
mark_as_advanced(USE_BOOST USE_EIGEN USE_XTENSOR)

option(HIGHFIVE_USE_BOOST "Enable Boost Support" ${USE_BOOST})
option(HIGHFIVE_USE_EIGEN "Enable Eigen testing" ${USE_EIGEN})
option(HIGHFIVE_USE_XTENSOR "Enable xtensor testing" ${USE_XTENSOR})
option(HIGHFIVE_UNIT_TESTS "Enable unit tests" ON)
option(HIGHFIVE_EXAMPLES "Compile examples" ON)
option(HIGHFIVE_PARALLEL_HDF5 "Enable Parallel HDF5 support" OFF)

# In deplomyents we probably don't want/cant have dynamic dependencies
option(HIGHFIVE_USE_INSTALL_DEPS "End applications by default use detected dependencies here" OFF)
mark_as_advanced(HIGHFIVE_USE_INSTALL_DEPS)

# HighFive as a sub-project and using the the same filename
# Define the HighFive INTERFACE target
include(${PROJECT_SOURCE_DIR}/CMake/HighFiveTargetExport.cmake)

# Search main dependencies (hdf5, boost, mpi) and link them to the HighFive INTERFACE target
include(${PROJECT_SOURCE_DIR}/CMake/HighFiveTargetDeps.cmake)

# Installation of headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
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

if(HIGHFIVE_USE_EIGEN)
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

if(HIGHFIVE_USE_XTENSOR)
  if(NOT COMPILER_SUPPORTS_CXX14)
    message(SEND_ERROR "C++ compiler does not support standard c++14, required to support xtensor.")
  else()
    set(CMAKE_CXX_STANDARD 14)
    find_package(xtensor REQUIRED)
    include_directories(SYSTEM $<TARGET_PROPERTY:xtensor,INTERFACE_INCLUDE_DIRECTORIES>)
    add_definitions(-DH5_USE_XTENSOR)
  endif()
endif()


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

