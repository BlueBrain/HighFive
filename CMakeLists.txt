project(HighFive)
cmake_minimum_required(VERSION 2.8)

enable_testing()

set(HIGHFIVE_VERSION_MAJOR 1)
set(HIGHFIVE_VERSION_MINOR 0)



list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
      ${PROJECT_SOURCE_DIR}/CMake/portability
      ${PROJECT_SOURCE_DIR}/CMake/packages
      ${PROJECT_SOURCE_DIR}/CMake/config)

include(DefineInstallationPaths)
include(ReleaseDebugAutoFlags)
include(CheckCXX11Portability)

include(BlueGenePortability)

option(UNIT_TESTS "enable unit tests" TRUE)
option(USE_BOOST "enable Boost Support" TRUE)
option(HIGHFIVE_EXAMPLES "Compile examples" TRUE)

find_package(HDF5 REQUIRED)

set(Boost_NO_BOOST_CMAKE TRUE)


if(USE_BOOST)
    find_package(Boost 1.41 REQUIRED COMPONENTS system unit_test_framework)
    add_definitions(-DH5_USE_BOOST )
endif()

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/highfive
		DESTINATION ${INCLUDE_INSTALL_DIR})

include_directories(${CMAKE_SOURCE_DIR}/include ${HDF5_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})

if(HIGHFIVE_EXAMPLES)
    add_subdirectory(src/examples)
endif()

add_subdirectory(tests/unit)
