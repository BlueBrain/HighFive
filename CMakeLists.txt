project(HighFive)
cmake_minimum_required(VERSION 2.8)

enable_testing()

set(HIGHFIVE_VERSION_MAJOR 1)
set(HIGHFIVE_VERSION_MINOR 2)

add_subdirectory(deps/git-cmake-format)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake
      ${CMAKE_CURRENT_SOURCE_DIR}/CMake/portability
      ${CMAKE_CURRENT_SOURCE_DIR}/CMake/packages
      ${CMAKE_CURRENT_SOURCE_DIR}/CMake/config)

include(DefineInstallationPaths)
include(ReleaseDebugAutoFlags)
include(CheckCXX11Portability)

include(BlueGenePortability)

option(UNIT_TESTS "enable unit tests" TRUE)
option(USE_BOOST "enable Boost Support" TRUE)
option(HIGHFIVE_EXAMPLES "Compile examples" TRUE)
option(HIGHFIVE_PARALLEL_HDF5 "Enable Parallel HDF5 support" FALSE)

if(HIGHFIVE_PARALLEL_HDF5)
set(HDF5_PREFER_PARALLEL TRUE)
endif()

find_package(HDF5 QUIET REQUIRED)

set(Boost_NO_BOOST_CMAKE TRUE)

if(USE_BOOST)
  find_package(Boost 1.41 QUIET REQUIRED COMPONENTS system unit_test_framework)
  add_definitions(-DH5_USE_BOOST )
endif()

if(HIGHFIVE_PARALLEL_HDF5)
  find_package(MPI)
endif()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/highfive
		DESTINATION ${INCLUDE_INSTALL_DIR})

include(PackageConfig)

if(HIGHFIVE_EXAMPLES)
  add_subdirectory(src/examples)
endif()

add_subdirectory(tests/unit)
