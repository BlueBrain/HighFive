
add_library(HighFive INTERFACE)

# Public headers

target_include_directories(HighFive
  INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include>")

# Add dependencies to "HighFive" target

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

# Generate HighFiveConfig.cmake

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/HighFiveConfig.cmake.in
    ${PROJECT_BINARY_DIR}/HighFiveConfig.cmake
  INSTALL_DESTINATION
    share/HighFive/CMake)

write_basic_package_version_file(
    HighFiveConfigVersion.cmake
  VERSION
    ${PROJECT_VERSION}
  COMPATIBILITY
    AnyNewerVersion)

install(
  FILES
    CMake/HighFiveTargetDeps.cmake
    ${PROJECT_BINARY_DIR}/HighFiveConfig.cmake
    ${PROJECT_BINARY_DIR}/HighFiveConfigVersion.cmake
  DESTINATION
    share/HighFive/CMake)

# Generate HighFiveTargets.cmake

install(
  TARGETS
    HighFive
  EXPORT
    HighFiveTargets
  INCLUDES DESTINATION
    include)

install(
  EXPORT
    HighFiveTargets
  FILE
    HighFiveTargets.cmake
  DESTINATION
    share/HighFive/CMake)

export(
  EXPORT
    HighFiveTargets
  FILE
    "${PROJECT_BINARY_DIR}/HighFiveTargets.cmake")

# Install static target "highfive_deps" if requested
# If not installed it is constructed when the user calls "find_package(HighFive)"

if (HIGHFIVE_INSTALL_STATIC)

  install(
    TARGETS
      highfive_deps
    EXPORT
      HighFiveStaticTargets
    INCLUDES DESTINATION
      include)

  install(
    EXPORT
      HighFiveStaticTargets
    FILE
      HighFiveStaticTargets.cmake
    DESTINATION
      share/HighFive/CMake)

  export(
    EXPORT
      HighFiveStaticTargets
    FILE
      "${PROJECT_BINARY_DIR}/HighFiveStaticTargets.cmake")

endif()
