foreach(test_name test_fixed_len_string_array)
  add_executable(${test_name} "${test_name}.cpp")

  target_link_libraries(${test_name} HighFive HighFiveWarnings Catch2::Catch2WithMain)
  catch_discover_tests(${test_name})

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${test_name} PRIVATE -Wno-deprecated-declarations)
  endif()
endforeach()
