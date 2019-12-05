get_filename_component(HighFive_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(CMakeFindDependencyMacro)

find_dependency(Boost QUIET)
find_dependency(xtensor QUIET)

if(NOT TARGET HighFive::HighFive)
    include("${HighFive_CMAKE_DIR}/HighFiveTargets.cmake")
endif()
