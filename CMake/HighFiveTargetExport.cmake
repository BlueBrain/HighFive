
# Define the HighFive INTERFACE library
add_library(libheaders INTERFACE)

target_include_directories(libheaders INTERFACE
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>")

# Combined HighFive
add_library(HighFive INTERFACE)
target_compile_definitions(HighFive INTERFACE MPI_NO_CPPBIND)  # No c++ bindings
target_link_libraries(HighFive INTERFACE libheaders libdeps)


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
    CMake/HighFiveTargetDeps.cmake
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  DESTINATION share/${PROJECT_NAME}/CMake)


# Provides IMPORTED targets when using this project from build/install trees.

# Specify targets to include in the HighFive Exports
install(TARGETS HighFive libheaders libdeps
        EXPORT HighFiveTargets)

# Generate & install the Export for the INSTALL_INTERFACE
install(EXPORT HighFiveTargets
        NAMESPACE HighFive_
        FILE HighFiveTargets.cmake
        DESTINATION share/${PROJECT_NAME}/CMake)

# Generate the Export for the BUILD_INTERACE (hardly used)
export(EXPORT HighFiveTargets
       FILE "${PROJECT_BINARY_DIR}/HighFiveTargets.cmake")
