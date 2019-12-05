add_library(HighFive INTERFACE)

# Public headers
target_include_directories(HighFive INTERFACE
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>")

# HDF5
foreach(target_name "${HDF5_C_TARGET}" "${HDF5_C_TARGET}-shared" "${HDF5_C_TARGET}-static")
  if(TARGET "${target_name}")
    target_link_libraries(HighFive INTERFACE "${target_name}")
    break()
  endif()
endforeach()

# BOOST
if(USE_BOOST)
  target_link_libraries(HighFive INTERFACE Boost::boost Boost::serialization)
  target_compile_definitions(HighFive INTERFACE -DH5_USE_BOOST)
endif()


# Generate ${PROJECT_NAME}Config.cmake

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/HighFiveConfig.cmake.in
  ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION share/${PROJECT_NAME}/CMake)

write_basic_package_version_file(
    ${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

install(FILES
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  DESTINATION share/${PROJECT_NAME}/CMake)


# Generate ${PROJECT_NAME}Targets.cmake;
# Provides IMPORTED targets when using this project from build/install trees.

# NOTE: the export file is ${PROJECT_NAME}Targets to support when HighFive
# is built within a 3rd-party project (X). Other projects can find and import
# X-Targets.cmake containing the HighFive target

# Specify targets to include in the HighFive Export
install(TARGETS HighFive
        EXPORT HighFiveTargets
        INCLUDES DESTINATION include)

# Generate & install the Export for the INSTALL_INTERFACE
install(EXPORT HighFiveTargets
        FILE ${PROJECT_NAME}Targets.cmake
        DESTINATION share/${PROJECT_NAME}/CMake)

# Generate the Export for the BUILD_INTERACE (hardly used)
export(EXPORT HighFiveTargets
       FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
