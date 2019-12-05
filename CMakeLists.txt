cmake_minimum_required(VERSION 3.1)

project(HighFive)

# find_package(Boost QUIET)
# find_package(xtensor QUIET)
find_package(HDF5 REQUIRED)

add_library(HighFive::HDF5 INTERFACE IMPORTED)
# target_link_libraries(HighFive::HDF5 INTERFACE Boost::boost)
set_target_properties(HighFive::HDF5 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${HDF5_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${HDF5_LIBRARIES}")


add_library(HighFive INTERFACE)

target_include_directories(HighFive INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_compile_features(HighFive INTERFACE cxx_std_11)

target_link_libraries(HighFive INTERFACE
    HighFive::HDF5)

include(GNUInstallDirs)
# include(CMakePackageConfigHelpers)

install(DIRECTORY include/highfive DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS HighFive EXPORT HighFive-targets)

install(EXPORT HighFive-targets
    FILE
        HighFiveTargets.cmake
    NAMESPACE
        HighFive::
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/HighFive)

install(DIRECTORY include/highfive DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
    FILES
        HighFiveConfig.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/HighFive)
