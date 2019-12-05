
cmake_minimum_required(VERSION 3.1)

# ----------------------
# Basic project settings
# ----------------------

project(HighFive)

option(HIGHFIVE_UNIT_TESTS "Enable unit tests" TRUE)
option(HIGHFIVE_EXAMPLES "Compile examples" TRUE)
option(HIGHFIVE_PARALLEL_HDF5 "Enable Parallel HDF5" FALSE)
option(HIGHFIVE_NO_BOOST "Disable Boost" FALSE)
option(HIGHFIVE_NO_EIGEN "Disable Eigen" FALSE)
option(HIGHFIVE_NO_XTENSOR "Disable xtensor" FALSE)

set(HIGHFIVE_NO_TARGET TRUE)

# --------------
# Find libraries
# --------------

if(HIGHFIVE_PARALLEL_HDF5)
  set(HDF5_PREFER_PARALLEL TRUE)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/HighFiveConfig.cmake)

# ---------------------
# Set target "HighFive"
# ---------------------

add_library(HighFive INTERFACE)

target_include_directories(HighFive INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_compile_features(HighFive INTERFACE cxx_std_11)

target_link_libraries(HighFive INTERFACE
    HighFive::HDF5
    HighFive::Boost
    HighFive::xtensor
    HighFive::Eigen)

# ----------------------------
# Export target "HighFive"
# Install headers in include/
# Install HighFiveConfig.cmake
# ----------------------------

include(GNUInstallDirs)

install(DIRECTORY include/highfive DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS HighFive EXPORT HighFive-targets)

install(EXPORT HighFive-targets
    FILE
        HighFiveTargets.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/HighFive)

install(DIRECTORY include/highfive DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
    FILES
        HighFiveConfig.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/HighFive)

# -------------------------
# Examples, Tests, and Docs
# -------------------------

if(HIGHFIVE_EXAMPLES)
  add_subdirectory(src/examples)
endif()

if(HIGHFIVE_UNIT_TESTS)
  enable_testing()
  add_subdirectory(tests/unit)
endif()

add_subdirectory(doc)
