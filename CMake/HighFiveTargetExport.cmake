# Generate ${PROJECT_NAME}Config.cmake
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

# Provides IMPORTED targets when using this project from build/install trees.
# Specify targets to include in the HighFive Exports
install(TARGETS HighFive
  EXPORT HighFiveTargets)

if(${HIGHFIVE_USE_INSTALL_DEPS})
  # Generate & install the Export for the INSTALL_INTERFACE
  install(EXPORT HighFiveTargets
    FILE HighFiveTargets.cmake
    DESTINATION share/${PROJECT_NAME}/CMake)

  configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/HighFiveConfigFixed.cmake.in
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION share/${PROJECT_NAME}/CMake)

  install(FILES
      ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
      ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION share/${PROJECT_NAME}/CMake)
else()
  configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/HighFiveConfigFlexible.cmake.in
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION share/${PROJECT_NAME}/CMake)

  install(FILES
      CMake/HighFiveTargetDeps.cmake
      ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
      ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION share/${PROJECT_NAME}/CMake)
endif()
