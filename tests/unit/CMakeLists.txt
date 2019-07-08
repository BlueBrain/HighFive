if(NOT HIGHFIVE_UNIT_TESTS OR NOT USE_BOOST)
  return()
endif()

include_directories(${PROJECT_SOURCE_DIR}/include ${HDF5_INCLUDE_DIRS})

if(NOT Boost_USE_STATIC_LIBS)
  add_definitions(-DBOOST_TEST_DYN_LINK=TRUE)
endif()

## base tests
foreach(test_name tests_high_five_base tests_high_five_easy)
  add_executable(${test_name} "${test_name}.cpp")
  target_link_libraries(${test_name}
                        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES}
                        ${HDF5_C_LIBRARIES})
  if(HIGHFIVE_PARALLEL_HDF5)
    target_link_libraries(${test_name} ${MPI_C_LIBRARIES})
  endif()

  target_compile_options(${test_name} PRIVATE "${HIGHFIVE_CPP_STD_FLAG}")

  add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()


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

