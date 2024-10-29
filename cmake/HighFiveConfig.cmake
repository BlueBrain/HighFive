include(CMakeFindDependencyMacro)

if(NOT DEFINED HIGHFIVE_FIND_HDF5)
  set(HIGHFIVE_FIND_HDF5 On)
endif()

if(HIGHFIVE_FIND_HDF5)
  find_dependency(HDF5)
endif()

if(NOT TARGET HighFive)
  include("${CMAKE_CURRENT_LIST_DIR}/HighFiveTargets.cmake")

  if(HDF5_IS_PARALLEL)
    find_dependency(MPI)
    target_link_libraries(HighFive::HighFive INTERFACE MPI::MPI_C MPI::MPI_CXX)
  endif()

  add_library(HighFive ALIAS HighFive::HighFive)
  add_library(HighFiveInclude ALIAS HighFive::Include)
endif()


