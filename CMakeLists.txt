cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

project(HighFive VERSION 3.0.0)
set(HIGHFIVE_VERSION_PRERELEASE 2)

message(WARNING "=================================================================\n"
                "HighFive development moved to:\n"
                "  github.com/highfive-devs/highfive\n"
                "=================================================================")

# Configure HighFive
# ------------------
option(HIGHFIVE_VERBOSE "Set logging level to verbose." OFF)

# Controls if HighFive classes are friends of each other.
#
# There are two compiler bugs that require incompatible choices. The
# GCC compiler bug [1] prevents us from writing:
#
#     template<class D>
#     friend class NodeTraits<D>;
#
# While a MSVC compiler bug [2] complains that it can't access a
# protected constructor, e.g., `HighFive::Object::Object`.
#
# Starting with `2.7.0` these friend declarations don't matter
# anymore. It's mearly a means of appeasing a compiler.
#
# The values of `HIGHFIVE_HAS_FRIEND_DECLARATIONS` are:
#   - that the macro is undefined.
#   - `0` which implies not adding the friend declarations.
#   - any non-zero integer, i.e. `1`, to add the friend declarations.
#
# Not defining the macro implies that it'll be set to `1` if MSVC is
# detected (or other compilers requiring the friend declarations).
#
# [1]: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52625
# [2]: https://developercommunity.visualstudio.com/t/MSVC-compiler-improperly-implements-N489/1516410
option(HIGHFIVE_HAS_FRIEND_DECLARATIONS "Enable additional friend declarations. Certain compiler require this On, others Off." OFF)
mark_as_advanced(HIGHFIVE_HAS_FRIEND_DECLARATIONS)

option(HIGHFIVE_FIND_HDF5 "Find and link with HDF5." On)

# Configure Tests & Examples
# --------------------------

# Internal variable that controls the default value for building
# optional things like tests, examples and docs.
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(HIGHFIVE_EXTRAS_DEFAULT ON)
else()
  set(HIGHFIVE_EXTRAS_DEFAULT OFF)
endif()

if (CMAKE_CXX_STANDARD GREATER_EQUAL 20)
  include(CheckIncludeFileCXX)
  CHECK_INCLUDE_FILE_CXX(span HIGHFIVE_TEST_SPAN_DEFAULT)
else()
  set(HIGHFIVE_TEST_SPAN_DEFAULT Off)
endif()

option(HIGHFIVE_UNIT_TESTS "Compile unit-tests" ${HIGHFIVE_EXTRAS_DEFAULT})
option(HIGHFIVE_EXAMPLES "Compile examples" ${HIGHFIVE_EXTRAS_DEFAULT})
option(HIGHFIVE_BUILD_DOCS "Build documentation" ${HIGHFIVE_EXTRAS_DEFAULT})

option(HIGHFIVE_TEST_SPAN "Enable testing std::span, requires C++20" ${HIGHFIVE_TEST_SPAN_DEFAULT})
option(HIGHFIVE_TEST_BOOST "Enable testing Boost features" OFF)
option(HIGHFIVE_TEST_BOOST_SPAN "Additionally, enable testing `boost::span`" OFF)
option(HIGHFIVE_TEST_EIGEN "Enable testing Eigen" OFF)
option(HIGHFIVE_TEST_OPENCV "Enable testing OpenCV" OFF)
option(HIGHFIVE_TEST_XTENSOR "Enable testing xtensor" OFF)
option(HIGHFIVE_TEST_HALF_FLOAT "Enable testing half-precision floats" OFF)

# TODO remove entirely.
option(HIGHFIVE_HAS_CONCEPTS "Print readable compiler errors w/ C++20 concepts" OFF)

set(HIGHFIVE_MAX_ERRORS 0 CACHE STRING "Maximum number of compiler errors.")
option(HIGHFIVE_HAS_WERROR "Convert warnings to errors." OFF)
option(HIGHFIVE_GLIBCXX_ASSERTIONS "Enable bounds check for STL." OFF)
# TODO these some magic to get a drop down menu in ccmake
set(HIGHFIVE_SANITIZER OFF CACHE STRING "Enable a group of sanitizers, requires compiler support. Supported: 'address' and 'undefined'.")
mark_as_advanced(HIGHFIVE_SANITIZER)

# Check compiler cxx_std requirements
# -----------------------------------
set(HIGHFIVE_CXX_STANDARD_DEFAULT 14)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD ${HIGHFIVE_CXX_STANDARD_DEFAULT})
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if(CMAKE_CXX_STANDARD EQUAL 98 OR CMAKE_CXX_STANDARD LESS ${HIGHFIVE_CXX_STANDARD_DEFAULT})
  message(FATAL_ERROR "HighFive needs to be compiled with at least C++${HIGHFIVE_CXX_STANDARD_DEFAULT}")
endif()

add_compile_definitions(HIGHFIVE_CXX_STD=${CMAKE_CXX_STANDARD})

# HighFive
# --------
add_library(HighFiveInclude INTERFACE)
add_library(HighFive::Include ALIAS HighFiveInclude)
set_target_properties(HighFiveInclude PROPERTIES EXPORT_NAME Include)

target_include_directories(HighFiveInclude INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

add_library(HighFive INTERFACE)
add_library(HighFive::HighFive ALIAS HighFive)

target_link_libraries(HighFive INTERFACE HighFive::Include)

if(HIGHFIVE_FIND_HDF5)
  find_package(HDF5 REQUIRED)
  target_link_libraries(HighFive INTERFACE HDF5::HDF5)
endif()

if(HDF5_IS_PARALLEL)
  find_package(MPI REQUIRED)
  target_link_libraries(HighFive
    INTERFACE
    $<BUILD_INTERFACE:MPI::MPI_C>
    $<BUILD_INTERFACE:MPI::MPI_CXX>
  )
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/highfive/H5Version.hpp.in
               ${CMAKE_CURRENT_SOURCE_DIR}/include/highfive/H5Version.hpp)

# Install
# -------
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/HighFiveConfigVersion.cmake
  VERSION ${PACKAGE_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION "include"
  PATTERN "*.in" EXCLUDE)

install(TARGETS HighFive HighFiveInclude EXPORT HighFiveTargets)
install(EXPORT HighFiveTargets
  FILE HighFiveTargets.cmake
  NAMESPACE HighFive::
  DESTINATION lib/cmake/HighFive
)

install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/HighFiveConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/HighFiveConfigVersion.cmake
  DESTINATION lib/cmake/HighFive
)

# Preparing local building (tests, examples)
# ------------------------------------------

if(HIGHFIVE_EXAMPLES OR HIGHFIVE_UNIT_TESTS)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/HighFiveWarnings.cmake)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/HighFiveFlags.cmake)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/HighFiveOptionalDependencies.cmake)
endif()

if(HIGHFIVE_EXAMPLES)
  add_subdirectory(src/examples)
endif()

if(HIGHFIVE_UNIT_TESTS)
  add_subdirectory(deps/catch2 EXCLUDE_FROM_ALL)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/catch2/contrib)
  enable_testing()
  add_subdirectory(tests/unit)
endif()

if(HIGHFIVE_BUILD_DOCS)
  add_subdirectory(doc)
endif()
