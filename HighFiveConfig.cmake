
get_filename_component(HighFive_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)


include(CMakeFindDependencyMacro)


if(NOT TARGET HighFive::HDF5)
    find_package(HDF5 REQUIRED)
    add_library(HighFive::HDF5 INTERFACE IMPORTED)
    set_target_properties(HighFive::HDF5 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${HDF5_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${HDF5_LIBRARIES}")
endif()


if(NOT TARGET HighFive::HighFive)
    include("${HighFive_CMAKE_DIR}/HighFiveTargets.cmake")
endif()
