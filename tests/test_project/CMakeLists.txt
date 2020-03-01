# This is a sample project to test integration via add_subdirectory and CMakeConfig
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

add_executable(read_write_bin "read_write_vector_dataset.cpp")
target_link_libraries(read_write_bin HighFive)

enable_testing()
add_test(NAME test_project COMMAND ${CMAKE_CURRENT_BINARY_DIR}/read_write_bin)
