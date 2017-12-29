project(HighFive)
cmake_minimum_required(VERSION 3.0)
include(ExternalProject)
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

file(GLOB HIGHFIVE_INC ${PROJECT_SOURCE_DIR}/include/highfive/*.hpp)
file(GLOB HIGHFIVE_INC_BITS ${PROJECT_SOURCE_DIR}/include/highfive/bits/*.hpp)

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
    FIND_PACKAGE(blosc)
    set(BLOSC_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/lib/blosc")
    set(BLOSC_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib/blosc")
    set(BLOSC_CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${BLOSC_INSTALL_DIR} -DPLUGIN_INSTALL_PATH=${BLOSC_INSTALL_DIR})
    ExternalProject_Add(hdf5-blosc
            PREFIX ${BLOSC_PREFIX}
            GIT_REPOSITORY https://github.com/CreRecombinase/hdf5-blosc.git
            INSTALL_DIR ${BLOSC_INSTALL_DIR}
            CMAKE_ARGS ${BLOSC_CMAKE_ARGS}
            )
    add_library(blosc_f SHARED IMPORTED)
    message("${CMAKE_SHARED_LIBRARY_PREFIX}")
    message("${CMAKE_SHARED_LIBRARY_SUFFIX}")
    message("${BLOSC_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}blosc_filter${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set_property(TARGET blosc_f PROPERTY IMPORTED_LOCATION ${BLOSC_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}blosc_filter${CMAKE_SHARED_LIBRARY_SUFFIX})
    add_dependencies(blosc_f hdf5-blosc)
    include_directories(${BLOSC_INSTALL_DIR}/include)
    ExternalProject_Add_StepTargets(hdf5-blosc build)
    set(hdf5-blosc_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/lib/blosc/include")
    include_directories(${hdf5-blosc_INCLUDE_DIRS})
endif ()
#add_subdirectory(lib/cxxopts)

# Absolute include used to avoid interaction with downstream projects including
# HighFive as a subproject and using the the same filename
include(${PROJECT_SOURCE_DIR}/CMake/PackageConfig.cmake)

if(HIGHFIVE_EXAMPLES)

    add_subdirectory(src/examples)
endif()

add_subdirectory(tests/unit)
add_subdirectory(doc)
#export(PACKAGE HighFive)

#install ( TARGETS  ... EXPORT <export_name> )
