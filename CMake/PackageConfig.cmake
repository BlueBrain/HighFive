add_library(HighFive INTERFACE)

# Public headers
target_include_directories(HighFive INTERFACE
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>")
target_include_directories(HighFive SYSTEM INTERFACE ${HDF5_INCLUDE_DIRS})
target_link_libraries(HighFive INTERFACE ${HDF5_LIBRARIES})
target_compile_definitions(HighFive INTERFACE ${HDF5_DEFINITIONS})

# MPI
if(HIGHFIVE_PARALLEL_HDF5 OR HDF5_IS_PARALLEL)
  target_include_directories(HighFive SYSTEM INTERFACE ${MPI_CXX_INCLUDE_PATH})
  target_link_libraries(HighFive INTERFACE ${MPI_CXX_LIBRARIES})
  if(CMAKE_VERSION VERSION_LESS 3.13)
    target_link_libraries(HighFive INTERFACE ${MPI_CXX_LINK_FLAGS})
  else()
    target_link_options(HighFive INTERFACE "SHELL:${MPI_CXX_LINK_FLAGS}")
  endif()
endif()

# BOOST
if(USE_BOOST)
  target_include_directories(HighFive SYSTEM INTERFACE ${Boost_INCLUDE_DIR})
  target_compile_definitions(HighFive INTERFACE -DH5_USE_BOOST)
  target_link_libraries(HighFive INTERFACE ${Boost_SERIALIZATION_LIBRARIES})
endif()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/highfive
  DESTINATION "include")

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/HighFiveConfig.cmake.in
  ${PROJECT_BINARY_DIR}/HighFiveConfig.cmake
  INSTALL_DESTINATION share/${PROJECT_NAME}/CMake)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  DESTINATION share/${PROJECT_NAME}/CMake)

# Generate ${PROJECT_NAME}Targets.cmake; is written after the CMake run
# succeeds. Provides IMPORTED targets when using this project from the install
# tree.
install(EXPORT HighFiveTargets FILE ${PROJECT_NAME}Targets.cmake
  DESTINATION share/${PROJECT_NAME}/CMake)

install(TARGETS HighFive EXPORT ${PROJECT_NAME}Targets
  INCLUDES DESTINATION include)

export(EXPORT HighFiveTargets
  FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
