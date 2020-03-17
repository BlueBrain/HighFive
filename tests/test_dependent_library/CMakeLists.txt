# This is a sample library to test integration via add_subdirectory and CMakeConfig
cmake_minimum_required(VERSION 3.1)

project(test_project VERSION 0.1)

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX11_STANDARD_COMPILE_OPTION "-std=c++11")  # For come compilers under cmake 3.1
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

option(USE_BUNDLED_HIGHFIVE "Use highfive from deps folder. Otherwise must be installed" ON)

if(USE_BUNDLED_HIGHFIVE)
    add_subdirectory("deps/HighFive" EXCLUDE_FROM_ALL)
else()
    find_package(HighFive REQUIRED)
endif()

add_library(simpleton SHARED "src/simpleton.cpp" "src/otherton.cpp")
target_include_directories(simpleton
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(simpleton PUBLIC HighFive)
set_property(TARGET simpleton PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(otherton STATIC "src/simpleton.cpp" "src/otherton.cpp")
target_include_directories(otherton
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(otherton PUBLIC HighFive)
set_property(TARGET otherton PROPERTY POSITION_INDEPENDENT_CODE OFF)

install(
    TARGETS simpleton otherton
    EXPORT simpletonTarget
    DESTINATION lib
    ARCHIVE DESTINATION lib)
install(EXPORT simpletonTarget DESTINATION lib)
