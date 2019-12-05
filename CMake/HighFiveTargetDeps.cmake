# Link against target system libs
# -------------------------------

# therefore making it possible to have new dependencies each build
if(NOT TARGET highfive_deps)
  add_library(highfive_deps INTERFACE)
else()
# Reset if imported
set_target_properties(highfive_deps PROPERTIES
  INTERFACE_COMPILE_DEFINITIONS ""
  INTERFACE_COMPILE_OPTIONS ""
  INTERFACE_INCLUDE_DIRECTORIES ""
  INTERFACE_LINK_LIBRARIES ""
  INTERFACE_LINK_OPTIONS ""
  INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ""
)
endif()


# HDF5
if(NOT DEFINED HDF5_C_LIBRARIES)
  set(HDF5_PREFER_PARALLEL ${HIGHFIVE_PARALLEL_HDF5})
  find_package(HDF5 REQUIRED)
endif()

if(HIGHFIVE_PARALLEL_HDF5 AND NOT HDF5_IS_PARALLEL)
  message(WARNING "Parallel HDF5 requested but libhdf5 doesnt support it")
endif()

target_include_directories(highfive_deps SYSTEM INTERFACE ${HDF5_INCLUDE_DIRS})
target_link_libraries(highfive_deps INTERFACE ${HDF5_C_LIBRARIES})
target_compile_definitions(highfive_deps INTERFACE ${HDF5_DEFINITIONS})

# Boost
if(USE_BOOST)
  find_package(Boost REQUIRED COMPONENTS system serialization)
  target_link_libraries(highfive_deps INTERFACE Boost::boost Boost::serialization)
  target_include_directories(highfive_deps INTERFACE ${Boost_INCLUDE_DIR})
endif()

# MPI
if(HIGHFIVE_PARALLEL_HDF5 OR HDF5_IS_PARALLEL)
  find_package(MPI REQUIRED)
  target_include_directories(highfive_deps SYSTEM INTERFACE ${MPI_CXX_INCLUDE_PATH})
  target_link_libraries(highfive_deps INTERFACE ${MPI_CXX_LIBRARIES})
  if(CMAKE_VERSION VERSION_LESS 3.13)
    target_link_libraries(highfive_deps INTERFACE ${MPI_CXX_LINK_FLAGS})
  else()
    target_link_options(highfive_deps INTERFACE "SHELL:${MPI_CXX_LINK_FLAGS}")
  endif()
endif()

# Propagate to HighFive
target_link_libraries(HighFive INTERFACE highfive_deps)
target_compile_definitions(HighFive INTERFACE MPI_NO_CPPBIND)  # No c++ bindings
if(USE_BOOST)
  target_compile_definitions(HighFive INTERFACE H5_USE_BOOST)
endif()
