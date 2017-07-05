if(NOT UNIT_TESTS OR NOT USE_BOOST)
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

add_executable(tests_high_five_bin ${tests_high_five_src})
target_link_libraries(tests_high_five_bin ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES}
  ${HDF5_C_LIBRARIES}  ${MPI_C_LIBRARIES})

add_test(NAME tests_high_five COMMAND tests_high_five_bin)

## if the compiler support C++11
## lets recompile the base tests with C++11 support
if(COMPILER_SUPPORTS_CXX11)

  add_executable(tests_high_five_cpp11_bin ${tests_high_five_src})
  target_link_libraries(tests_high_five_cpp11_bin
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} ${HDF5_C_LIBRARIES}
    ${MPI_C_LIBRARIES})
  target_compile_options(tests_high_five_cpp11_bin PRIVATE "-std=c++11")

  add_test(NAME tests_high_five_cpp11 COMMAND tests_high_five_cpp11_bin)

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

