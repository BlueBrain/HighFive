# This is an example of an application/executable using HighFive. It
# demonstrates the different vendoring strategies and targets provided by
# HighFive.

cmake_minimum_required(VERSION 3.14)
project(Hi5Application VERSION 0.1)

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

set(INTEGRATION_STRATEGY "short" CACHE STRING "Use 'Include' for HighFive::Include, 'full' for HighFive::HighFive, 'short' for HighFive.")
set(VENDOR_STRATEGY "submodule" CACHE STRING "Use 'submodule' for Git submodules, 'fetch_content' for FetchContent, 'external' for `find_package`.")
option(USE_STATIC_HDF5 "Link against static HDF5" OFF)
option(USE_BOOST "Simulates an application using Boost" OFF)

# Controlling HDF5 features is done by directly setting the HDF5 flags. The
# interesting ones are probably:
#  * HDF5_USE_STATIC_LIBRARIES
#  * HDF5_PREFER_PARALLEL
if(USE_STATIC_HDF5)
  set(HDF5_USE_STATIC_LIBRARIES On)
else()
  set(HDF5_USE_STATIC_LIBRARIES Off)
endif()

if(${INTEGRATION_STRATEGY} STREQUAL "bailout")
  set(HIGHFIVE_FIND_HDF5 Off)
endif()

if(${VENDOR_STRATEGY} STREQUAL "submodule")
  # When vendoring via a Git submodule, this is the correct
  # line to include HighFive.
  add_subdirectory("deps/HighFive" EXCLUDE_FROM_ALL)
elseif(${VENDOR_STRATEGY} STREQUAL "fetch_content")
  include(FetchContent)
  FetchContent_Declare(HighFive
    GIT_REPOSITORY $ENV{HIGHFIVE_GIT_REPOSITORY}
    GIT_TAG $ENV{HIGHFIVE_GIT_TAG}
  )
  FetchContent_MakeAvailable(HighFive)
elseif(${VENDOR_STRATEGY} STREQUAL "external")
  # When HighFive is installed like regular software and then "found", do the
  # following:
  find_package(HighFive REQUIRED)
endif()

add_executable(Hi5Application "hi5_application.cpp")

if(   ${INTEGRATION_STRATEGY} STREQUAL "Include"
   OR ${INTEGRATION_STRATEGY} STREQUAL "bailout")
  # Only add `-I${HIGHFIVE_DIR}/include`.
  target_link_libraries(Hi5Application PUBLIC HighFive::Include)

  # Now link to HDF5 in whatever fashion you desire.
  find_package(HDF5 REQUIRED)
  target_link_libraries(Hi5Application PUBLIC HDF5::HDF5)

  # You might need to take care of MPI.
  find_package(MPI REQUIRED)
  target_link_libraries(Hi5Application PUBLIC MPI::MPI_C MPI::MPI_CXX)
elseif(${INTEGRATION_STRATEGY} STREQUAL "short")
  # Highest chance of being backwards compatible with v2.
  target_link_libraries(Hi5Application PUBLIC HighFive)
elseif(${INTEGRATION_STRATEGY} STREQUAL "full")
  target_link_libraries(Hi5Application PUBLIC HighFive::HighFive)
endif()

if(USE_BOOST)
  find_package(Boost REQUIRED)
  target_link_libraries(Hi5Application PUBLIC Boost::headers)
  target_compile_definitions(Hi5Application PUBLIC HI5_APPLICATION_HAS_BOOST=1)
endif()

if(USE_STATIC_HDF5)
  find_package(ZLIB REQUIRED)
  target_link_libraries(${target} PUBLIC ZLIB::ZLIB)
endif()

# Install
# -------
install(TARGETS Hi5Application RUNTIME DESTINATION bin)

enable_testing()
add_test(NAME test_hi5_application COMMAND ${CMAKE_CURRENT_BINARY_DIR}/Hi5Application)
