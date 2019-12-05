
get_filename_component(HighFive_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

include(CMakeFindDependencyMacro)

# ---------------------------
# Set target "HighFive::HDF5"
# ---------------------------

if(NOT TARGET HighFive::HDF5)

    find_package(HDF5 REQUIRED)

    add_library(HighFive::HDF5 INTERFACE IMPORTED)

    set_target_properties(HighFive::HDF5 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${HDF5_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${HDF5_LIBRARIES}")

endif()

# ----------------------------
# Set target "HighFive::Boost"
# ----------------------------

if(NOT TARGET HighFive::Boost)

    add_library(HighFive::Boost INTERFACE IMPORTED)

    if(NOT HIGHFIVE_NO_BOOST)

        find_package(Boost COMPONENTS system serialization QUIET)

        if(Boost_FOUND)
            target_link_libraries(HighFive::Boost INTERFACE
                Boost::boost
                Boost::system
                Boost::serialization)
            target_compile_definitions(HighFive::Boost INTERFACE "H5_USE_BOOST")
        endif()

    endif()

endif()

# ------------------------------
# Set target "HighFive::xtensor"
# ------------------------------

if(NOT TARGET HighFive::xtensor)

    add_library(HighFive::xtensor INTERFACE IMPORTED)

    if(NOT HIGHFIVE_NO_XTENSOR)

        find_package(xtensor QUIET)

        if(xtensor_FOUND)
            target_link_libraries(HighFive::xtensor INTERFACE xtensor)
            target_compile_definitions(HighFive::xtensor INTERFACE "H5_USE_XTENSOR")
        endif()

    endif()

endif()

# ----------------------------
# Set target "HighFive::Eigen"
# ----------------------------

if(NOT TARGET HighFive::Eigen)

    add_library(HighFive::Eigen INTERFACE IMPORTED)

    if(NOT HIGHFIVE_NO_EIGEN)

        find_package(Eigen3 QUIET)

        if(NOT Eigen3_FOUND)

            find_package(PkgConfig QUIET)

            if (PkgConfig_FOUND)
                pkg_check_modules(EIGEN3 QUIET eigen3)
            endif()

        endif()

        set_target_properties(HighFive::Eigen PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${EIGEN3_INCLUDE_DIR}")

    endif()

endif()

# ---------------------
# Set target "HighFive"
# ---------------------

if(NOT TARGET HighFive AND NOT HIGHFIVE_NO_TARGET)
    include("${HighFive_CMAKE_DIR}/HighFiveTargets.cmake")
endif()
