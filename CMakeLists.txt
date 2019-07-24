cmake_minimum_required(VERSION 3.0)
project(HighFive VERSION 2.0)


list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
  ${PROJECT_SOURCE_DIR}/CMake/portability
  ${PROJECT_SOURCE_DIR}/CMake/packages
  ${PROJECT_SOURCE_DIR}/CMake/config)

include(DefineInstallationPaths)
include(ReleaseDebugAutoFlags)
include(CheckCXXStandardSupport)
include(BlueGenePortability)

if(NOT COMPILER_SUPPORTS_CXX11)
    message(FATAL_ERROR "C++ compiler does not support standard c++11.")
endif()

option(HIGHFIVE_UNIT_TESTS "enable unit tests" TRUE)
option(USE_BOOST "enable Boost Support" TRUE)
option(USE_EIGEN "enable Eigen testing" FALSE)
option(USE_XTENSOR "enable xtensor testing" FALSE)
option(HIGHFIVE_EXAMPLES "Compile examples" TRUE)
option(HIGHFIVE_PARALLEL_HDF5 "Enable Parallel HDF5 support" FALSE)


if(HIGHFIVE_PARALLEL_HDF5)
    set(HDF5_PREFER_PARALLEL TRUE)
endif()
find_package(HDF5 REQUIRED)


set(Boost_NO_BOOST_CMAKE TRUE)
if(USE_BOOST)
  set(BOOST_REQ_COMPONENTS system)
  if(HIGHFIVE_UNIT_TESTS)
      list(APPEND BOOST_REQ_COMPONENTS unit_test_framework)
  endif()
  find_package(Boost REQUIRED COMPONENTS ${BOOST_REQ_COMPONENTS})
  add_definitions(-DH5_USE_BOOST)
  include_directories(SYSTEM ${Boost_INCLUDE_DIR})
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
        set(HIGHFIVE_CPP_STD_FLAG "${CMAKE_CXX14_STANDARD_COMPILE_OPTION}" CACHE STRING "flag to use to enforce c++ standard (default:c++11)")
        find_package(xtensor REQUIRED)
        include_directories(SYSTEM $<TARGET_PROPERTY:xtensor,INTERFACE_INCLUDE_DIRECTORIES>)
        add_definitions(-DH5_USE_XTENSOR)
    endif()
else()
   set(HIGHFIVE_CPP_STD_FLAG "${CMAKE_CXX11_STANDARD_COMPILE_OPTION}" CACHE STRING "flag to use to enforce c++ standard (default: c++11)")
endif()


if(HIGHFIVE_PARALLEL_HDF5)
  find_package(MPI)
endif()

if(NOT COMPILER_SUPPORTS_CXX11)
    message(FATAL "HighFive version >=2.0 requires c++ standard >= c++11")
endif()

# Absolute include used to avoid interaction with downstream projects including
# HighFive as a subproject and using the the same filename
include(${PROJECT_SOURCE_DIR}/CMake/PackageConfig.cmake)

if(HIGHFIVE_EXAMPLES)
  add_subdirectory(src/examples)
endif()

if(HIGHFIVE_UNIT_TESTS)
  if (NOT USE_BOOST)
    message(FATAL_ERROR "Unit tests require boost. Please disable HIGHFIVE_UNIT_TESTS or enable USE_BOOST")
  endif()
  enable_testing()
  add_subdirectory(tests/unit)
endif()

add_subdirectory(doc)
