cmake_minimum_required(VERSION 3.14)
project(TestHi5Dependent VERSION 0.1)

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

add_executable(test_hi5_dependent test_dependent_library.cpp)

if(NOT USE_BOOST)
  find_package(Hi5Dependent REQUIRED)
  target_link_libraries(test_hi5_dependent PUBLIC Hi5Dependent::Read Hi5Dependent::Write)
else()
  find_package(Hi5Dependent REQUIRED COMPONENTS boost)
  target_link_libraries(test_hi5_dependent PUBLIC Hi5Dependent::Read Hi5Dependent::Write)
  target_link_libraries(test_hi5_dependent PUBLIC Hi5Dependent::Boost)
endif()

enable_testing()
add_test(NAME run_test_hi5_dependent COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_hi5_dependent)
