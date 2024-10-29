cmake_minimum_required(VERSION 3.14)
project(TestHi5Dependent VERSION 0.1)

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

set(VENDOR_STRATEGY "submodule" CACHE STRING "Use 'submodule' for Git submodules, 'fetch_content' for FetchContent, 'external' for `find_package`, 'none' for making to attempt at finding HighFive.")

add_executable(test_hi5_dependent test_dependent_library.cpp)

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

if(NOT ${VENDOR_STRATEGY} STREQUAL "none")
  target_link_libraries(test_hi5_dependent PUBLIC HighFive::HighFive)
endif()

if(NOT USE_BOOST)
  find_package(Hi5Dependent REQUIRED)
  target_link_libraries(test_hi5_dependent PUBLIC Hi5Dependent::Read Hi5Dependent::Write)
else()
  find_package(Hi5Dependent REQUIRED COMPONENTS boost)
  target_link_libraries(test_hi5_dependent PUBLIC Hi5Dependent::Read Hi5Dependent::Write)
  target_link_libraries(test_hi5_dependent PUBLIC Hi5Dependent::Boost)
endif()

enable_testing()
add_test(NAME run_test_hi5_dependent COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_hi5_dependent)
