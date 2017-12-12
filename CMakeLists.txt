project(HighFive)
cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 17)
enable_testing()

set(HIGHFIVE_VERSION_MAJOR 1)
set(HIGHFIVE_VERSION_MINOR 5)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
  ${PROJECT_SOURCE_DIR}/CMake/portability
  ${PROJECT_SOURCE_DIR}/CMake/packages
  ${PROJECT_SOURCE_DIR}/CMake/config)

include(DefineInstallationPaths)
include(ReleaseDebugAutoFlags)
include(CheckCXX11Portability)

include(BlueGenePortability)

option(USE_BLOSC "enable blosc/compression tests" TRUE)
option(HIGHFIVE_UNIT_TESTS "enable unit tests" TRUE)
option(USE_EIGEN "enable Eigen" TRUE)
option(USE_BOOST "enable Boost Support" TRUE)
option(HIGHFIVE_EXAMPLES "Compile examples" TRUE)
option(HIGHFIVE_PARALLEL_HDF5 "Enable Parallel HDF5 support" FALSE)


set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

if(HIGHFIVE_PARALLEL_HDF5)
set(HDF5_PREFER_PARALLEL TRUE)
endif()

find_package(HDF5 QUIET REQUIRED)

set(Boost_NO_BOOST_CMAKE TRUE)

if(USE_BOOST)
  find_package(Boost QUIET REQUIRED COMPONENTS system unit_test_framework)
  add_definitions(-DH5_USE_BOOST )
  include_directories(SYSTEM ${Boost_INCLUDE_DIR})
endif()

if (USE_EIGEN)
    find_package(Eigen3 3.3 REQUIRED NO_MODULE)
  add_definitions(-DH5_USE_EIGEN)
endif ()

if(HIGHFIVE_PARALLEL_HDF5)
    find_package(MPI REQUIRED)
    message("MPI_C_COMPILER='${MPI_C_COMPILER}'")
    message("HDF5_C_LIBRARIES='${HDF5_C_LIBRARIES}'")
    message("HDF5_IS_PARALLEL='${HDF5_IS_PARALLEL}'")
endif()

if (USE_BLOSC)
    add_subdirectory(lib/hdf5-blosc)
endif ()
add_subdirectory(lib/cxxopts)

# Absolute include used to avoid interaction with downstream projects including
# HighFive as a subproject and using the the same filename
include(${PROJECT_SOURCE_DIR}/CMake/PackageConfig.cmake)

if(HIGHFIVE_EXAMPLES)
  add_subdirectory(src/examples)
endif()

add_subdirectory(tests/unit)
add_subdirectory(doc)
