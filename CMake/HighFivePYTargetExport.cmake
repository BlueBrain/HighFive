# pybind11
add_subdirectory(pybind11)

# Combined HighFive
pybind11_add_module(HighFivePY src/py/HighFivePY.cpp)

# HDF5
if(NOT DEFINED HDF5_C_LIBRARIES)
  set(HDF5_NO_FIND_PACKAGE_CONFIG_FILE TRUE)  # Consistency
  set(HDF5_USE_STATIC_LIBRARIES FALSE)  # Only shared library is needed
  set(HDF5_PREFER_PARALLEL ${HIGHFIVE_PARALLEL_HDF5})
  find_package(HDF5 REQUIRED)
endif()

target_include_directories(HighFivePY SYSTEM INTERFACE ${HDF5_INCLUDE_DIRS})
target_link_libraries(HighFivePY INTERFACE ${HDF5_C_LIBRARIES})
target_compile_definitions(HighFivePY INTERFACE ${HDF5_DEFINITIONS})

# Eigen
if (NOT Eigen3_FOUND)
  find_package(Eigen3 REQUIRED)
  message("EIGEN3_INCLUDE_DIR: ${EIGEN3_INCLUDE_DIR}")
  target_include_directories(HighFivePY PRIVATE ${EIGEN3_INCLUDE_DIR})
endif()

target_link_libraries(HighFivePY PRIVATE HighFive)
