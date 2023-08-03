include(CTest)
include(Catch)
include(HighFiveWarnings)

if(MSVC)
  add_definitions(/bigobj)
endif()

## Base tests
foreach(test_name tests_high_five_base tests_high_five_multi_dims tests_high_five_easy test_all_types)
  add_executable(${test_name} "${test_name}.cpp")
  target_link_libraries(${test_name} HighFive HighFiveWarnings Catch2::Catch2WithMain)
  catch_discover_tests(${test_name})
endforeach()

if(HIGHFIVE_PARALLEL_HDF5)
  include(TestHelpers)
  set(tests_parallel_src "tests_high_five_parallel.cpp")

  ## parallel MPI tests
  add_executable(tests_parallel_bin ${tests_parallel_src})
  target_link_libraries(tests_parallel_bin HighFive HighFiveWarnings Catch2::Catch2)

  # We need to patch in a call to `mpirun` or equivalent when using
  # parallel tests. Somehow, this is not foreseen in Catch2, modify the
  # test detection script and fail if the execution method needs updating.
  set(original_catch_script "${_CATCH_DISCOVER_TESTS_SCRIPT}")
  set(patched_catch_script "${CMAKE_CURRENT_BINARY_DIR}/patched_catch_test_discovery.cmake")
  file(READ "${original_catch_script}" original_catch_script_contents)
  string(REGEX REPLACE
         "(add_command\\(add_test.*TEST_EXECUTOR})"
         "\\1 ${TEST_MPI_EXEC_PREFIX_DEFAULT} -n 2"
         modified_catch_script_contents
         "${original_catch_script_contents}")
  if(original_catch_script_contents STREQUAL modified_catch_script_contents)
    message(FATAL_ERROR "Failed to modify Catch2 test execution")
  endif()
  file(WRITE "${patched_catch_script}" "${modified_catch_script_contents}")
  set(_CATCH_DISCOVER_TESTS_SCRIPT "${patched_catch_script}")
  catch_discover_tests(tests_parallel_bin)
  set(_CATCH_DISCOVER_TESTS_SCRIPT "${original_catch_script}")
endif()

option(HIGHFIVE_TEST_SINGLE_INCLUDES "Enable testing single includes" FALSE)

if(HIGHFIVE_TEST_SINGLE_INCLUDES)
    file(GLOB public_headers LIST_DIRECTORIES false RELATIVE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/include/highfive/*.hpp)
    foreach(PUBLIC_HEADER ${public_headers})
        get_filename_component(CLASS_NAME ${PUBLIC_HEADER} NAME_WE)
        configure_file(tests_import_public_headers.cpp "tests_${CLASS_NAME}.cpp" @ONLY)
        add_executable("tests_include_${CLASS_NAME}" "${CMAKE_CURRENT_BINARY_DIR}/tests_${CLASS_NAME}.cpp")
        target_link_libraries("tests_include_${CLASS_NAME}" HighFive HighFiveWarnings)
    endforeach()
endif()
