# Do NOT document the available components and targets. Guessing CMake targets
# and components is an essential part of the game. This library for example has
# one component called `boost` and three targets. Use
#
#   # Without Boost:
#   find_package(Hi5Dependent REQUIRED)
#   target_link_libraries(foo PUBLIC Hi5Dependent::Read Hi5Dependent::Write)
#
#   # With Boost:
#   find_package(Hi5Dependent REQUIRED COMPONENTS boost)
#   target_link_libraries(foo PUBLIC Hi5Dependent::Read Hi5Dependent::Write)
#   target_link_libraries(foo PUBLIC Hi5Dependent::Boost)

cmake_minimum_required(VERSION 3.14)
project(Hi5Dependent VERSION 0.1)

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

set(INTEGRATION_STRATEGY "short" CACHE STRING "Use 'Include' for HighFive::Include, 'full' for HighFive::HighFive, 'short' for HighFive.")
option(USE_STATIC_HDF5 "Link against static HDF5" OFF)
option(USE_BOOST "Build '${PROJECT_NAME}' with optional Boost dependency." OFF)

if(USE_STATIC_HDF5)
  set(HDF5_USE_STATIC_LIBRARIES On)
else()
  set(HDF5_USE_STATIC_LIBRARIES Off)
endif()

if(${INTEGRATION_STRATEGY} STREQUAL "bailout")
  set(HIGHFIVE_FIND_HDF5 Off)
endif()

# Since any project depending on 'Hi5Dependent' also needs HighFive, it doesn't
# make sense to vendor HighFive. Therefore, use
find_package(HighFive REQUIRED)

# For demonstration purposes it consists of a shared and static library
add_library(${PROJECT_NAME}Write SHARED "src/hi5_dependent/write_vector.cpp")
add_library(${PROJECT_NAME}::Write ALIAS ${PROJECT_NAME}Write)
set_target_properties(${PROJECT_NAME}Write PROPERTIES EXPORT_NAME Write)

add_library(${PROJECT_NAME}Read STATIC "src/hi5_dependent/read_vector.cpp")
add_library(${PROJECT_NAME}::Read ALIAS ${PROJECT_NAME}Read)
set_target_properties(${PROJECT_NAME}Read PROPERTIES EXPORT_NAME Read)

set(Hi5DependentCoreTargets ${PROJECT_NAME}Write ${PROJECT_NAME}Read)
set(Hi5DependentAllTargets ${Hi5DependentCoreTargets})

# ... and two more for demonstrating an optional dependency (on Boost).
if(USE_BOOST)
  add_library(${PROJECT_NAME}Boost SHARED "src/hi5_dependent/boost.cpp")
  add_library(${PROJECT_NAME}::Boost ALIAS ${PROJECT_NAME}Boost)
  set_target_properties(${PROJECT_NAME}Boost PROPERTIES EXPORT_NAME Boost)

  find_package(Boost REQUIRED)
  target_link_libraries(${PROJECT_NAME}Boost PUBLIC Boost::headers)
  target_compile_definitions(${PROJECT_NAME}Boost PUBLIC HI5_DEPENDENT_HAS_BOOST=1)

  list(APPEND Hi5DependentAllTargets ${PROJECT_NAME}Boost)
endif()

foreach(target IN LISTS Hi5DependentAllTargets)
  target_include_directories(${target}
      PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      PUBLIC $<INSTALL_INTERFACE:include>
  )

  # Remember to pick one. Writing out all variation serves testing and
  # demonstration purposes only. Minimizing lines of code is probably a good
  # strategy.
  if(   ${INTEGRATION_STRATEGY} STREQUAL "Include"
     OR ${INTEGRATION_STRATEGY} STREQUAL "bailout")
    target_link_libraries(${target} PUBLIC HighFive::Include)

    find_package(HDF5 REQUIRED)
    target_link_libraries(${target} PUBLIC HDF5::HDF5)

    find_package(MPI REQUIRED)
    target_link_libraries(${target} PUBLIC MPI::MPI_C MPI::MPI_CXX)
  elseif(${INTEGRATION_STRATEGY} STREQUAL "short")
    target_link_libraries(${target} PUBLIC HighFive)
  elseif(${INTEGRATION_STRATEGY} STREQUAL "full")
    target_link_libraries(${target} PUBLIC HighFive::HighFive)
  endif()

  if(USE_STATIC_HDF5)
    find_package(ZLIB REQUIRED)
    target_link_libraries(${target} PUBLIC ZLIB::ZLIB)
  endif()
endforeach()

# Install
# -------
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PACKAGE_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION "include")

install(TARGETS ${PROJECT_NAME}Read ${PROJECT_NAME}Write EXPORT ${PROJECT_NAME}Targets)
install(EXPORT ${PROJECT_NAME}Targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION cmake
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake
  @ONLY
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake
  DESTINATION cmake
)

if(USE_BOOST)
  install(TARGETS ${PROJECT_NAME}Boost EXPORT ${PROJECT_NAME}BoostTargets)
  install(EXPORT ${PROJECT_NAME}BoostTargets
    FILE ${PROJECT_NAME}BoostTargets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION cmake
  )
endif()
