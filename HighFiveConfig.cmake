
get_filename_component(HighFive_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)


include(CMakeFindDependencyMacro)


if(NOT TARGET HighFive::HDF5)
    find_package(HDF5 REQUIRED)
    add_library(HighFive::HDF5 INTERFACE IMPORTED)
    set_target_properties(HighFive::HDF5 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${HDF5_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${HDF5_LIBRARIES}")
endif()


if(NOT TARGET HighFive::Boost)
    find_package(Boost QUIET)
    add_library(HighFive::Boost INTERFACE IMPORTED)
    if(Boost_FOUND)
        target_link_libraries(HighFive::Boost INTERFACE Boost::boost)
        target_compile_definitions(HighFive::Boost INTERFACE "H5_USE_BOOST")
    endif()
endif()


if(NOT TARGET HighFive::xtensor)
    find_package(xtensor QUIET)
    add_library(HighFive::xtensor INTERFACE IMPORTED)
    if(xtensor_FOUND)
        target_link_libraries(HighFive::xtensor INTERFACE xtensor)
        target_compile_definitions(HighFive::xtensor INTERFACE "H5_USE_XTENSOR")
    endif()
endif()


if(NOT TARGET HighFive::Eigen)
    find_package(Eigen3 QUIET)
    add_library(HighFive::Eigen INTERFACE IMPORTED)
    set_target_properties(HighFive::Eigen PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${EIGEN3_INCLUDE_DIR}")
endif()


if(NOT TARGET HighFive::HighFive)
    include("${HighFive_CMAKE_DIR}/HighFiveTargets.cmake")
endif()
