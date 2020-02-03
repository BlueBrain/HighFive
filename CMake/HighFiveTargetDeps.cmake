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
  set(HDF5_NO_FIND_PACKAGE_CONFIG_FILE TRUE)  # Consistency
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
if(HIGHFIVE_USE_BOOST)
  set(Boost_NO_BOOST_CMAKE TRUE)  # Consistency
  find_package(Boost REQUIRED COMPONENTS system serialization)
  # Dont use imported targets yet, not avail before cmake 3.5
  target_include_directories(highfive_deps SYSTEM INTERFACE ${Boost_INCLUDE_DIR})
  target_compile_definitions(highfive_deps INTERFACE BOOST_ALL_NO_LIB H5_USE_BOOST)
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

# Ensure we activate at least C++11
if(NOT DEFINED CMAKE_CXX_STANDARD)
  if(CMAKE_VERSION VERSION_LESS "3.1")
    message(WARNING "HighFive requires at least c++11. You may need to set CMAKE_CXX_STANDARD.")
  else()
    target_compile_features(HighFive INTERFACE cxx_std_11)
  endif()
endif()