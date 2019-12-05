
cmake_minimum_required(VERSION 3.1)

project(HighFive)

find_package(Boost QUIET)
find_package(xtensor QUIET)
find_package(Eigen3 QUIET)
find_package(HDF5 REQUIRED)

add_library(HighFive::HDF5 INTERFACE IMPORTED)
add_library(HighFive::Boost INTERFACE IMPORTED)
add_library(HighFive::xtensor INTERFACE IMPORTED)
add_library(HighFive::Eigen INTERFACE IMPORTED)

# ---------------------------
# Set target "HighFive::HDF5"
# ---------------------------

set_target_properties(HighFive::HDF5 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${HDF5_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${HDF5_LIBRARIES}")

# ----------------------------
# Set target "HighFive::Boost"
# ----------------------------

if(Boost_FOUND)
    target_link_libraries(HighFive::Boost INTERFACE Boost::boost)
    target_compile_definitions(HighFive::Boost INTERFACE "H5_USE_BOOST")
endif()

# ------------------------------
# Set target "HighFive::xtensor"
# ------------------------------

if(xtensor_FOUND)
    target_link_libraries(HighFive::xtensor INTERFACE xtensor)
    target_compile_definitions(HighFive::xtensor INTERFACE "H5_USE_XTENSOR")
endif()

# ----------------------------
# Set target "HighFive::Eigen"
# ----------------------------

if(Eigen3_FOUND)
    set_target_properties(HighFive::Eigen PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${EIGEN3_INCLUDE_DIR}")
endif()

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

# ------------------------------------------------
# Export target "HighFive" -> "HighFive::HighFive"
# Install headers
# Install CMake support
# ------------------------------------------------

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
