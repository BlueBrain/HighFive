if(NOT HIGHFIVE_UNIT_TESTS OR NOT USE_BOOST)
  return()
endif()

include_directories(${PROJECT_SOURCE_DIR}/include ${HDF5_INCLUDE_DIRS})

if(NOT MSVC)
  # silent boost warnings
  add_definitions(-Wno-unused-parameter -Wno-unused-local-typedef)
endif()

if(NOT Boost_USE_STATIC_LIBS)
  add_definitions(-DBOOST_TEST_DYN_LINK=TRUE)
endif()

## base tests
file(GLOB tests_high_five_src "*base.cpp")

add_executable(tests_high_five_cpp11_bin ${tests_high_five_src})
target_link_libraries(tests_high_five_cpp11_bin
${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} ${HDF5_C_LIBRARIES})

if(HIGHFIVE_PARALLEL_HDF5)
  target_link_libraries(tests_high_five_cpp11_bin ${MPI_C_LIBRARIES})
endif()

target_compile_options(tests_high_five_cpp11_bin PRIVATE "${HIGHFIVE_CPP_STD_FLAG}")

add_test(NAME tests_high_five_cpp11 COMMAND tests_high_five_cpp11_bin)

if(USE_EIGEN)
  list(APPEND CMAKE_MODULE_PATH "/usr/share/cmake-2.8/Modules/")
  find_package(Eigen3 REQUIRED)
  include_directories(${EIGEN3_INCLUDE_DIR})
  add_definitions(-DHIGHFIVE_EIGEN)
endif()

# disable xtensor-testing for compilers that do not support C++14
if(USE_XTENSOR)
  if(CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
    message(STATUS "Use g++ 4.9 or greater to enable xtensor testing.")
    set(USE_XTENSOR FALSE)
  endif()
endif()

if(USE_XTENSOR)
  find_package(xtl REQUIRED)
  find_package(xtensor REQUIRED)
  target_link_libraries(tests_high_five_cpp11_bin xtl xtensor)
  add_definitions(-DHIGHFIVE_XTENSOR)
endif()

if(HIGHFIVE_PARALLEL_HDF5)

  include(TestHelpers)

  ## parallel MPI tests
  file(GLOB tests_parallel_src "*parallel.cpp")

  add_executable(tests_parallel_bin ${tests_parallel_src})
  target_link_libraries(tests_parallel_bin
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} ${HDF5_C_LIBRARIES}
    ${MPI_C_LIBRARIES})
  add_test(NAME tests_parallel COMMAND ${TEST_MPI_EXEC_PREFIX_DEFAULT} -n 8
    ${CMAKE_CURRENT_BINARY_DIR}/tests_parallel_bin)

endif()

