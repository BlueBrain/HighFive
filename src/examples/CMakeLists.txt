set(core_examples
  ${CMAKE_CURRENT_SOURCE_DIR}/compound_types.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/broadcasting_arrays.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/create_attribute_string_integer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/create_dataset_double.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/create_datatype.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/create_extensible_dataset.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/create_large_attribute.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/create_page_allocated_files.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/readme_snippet.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/read_write_dataset_string.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/read_write_raw_ptr.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/read_write_single_scalar.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/read_write_std_strings.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/read_write_vector_dataset.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/read_write_vector_dataset_references.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/renaming_objects.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/select_by_id_dataset_cpp11.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/select_partial_dataset_cpp11.cpp
)

set(span_examples
  ${CMAKE_CURRENT_SOURCE_DIR}/read_write_std_span.cpp
)

set(easy_examples
  ${CMAKE_CURRENT_SOURCE_DIR}/easy_attribute.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/easy_dumpoptions.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/easy_load_dump.cpp
)

set(boost_examples
  ${CMAKE_CURRENT_SOURCE_DIR}/boost_multi_array_2D.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/boost_multiarray_complex.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/boost_ublas_double.cpp
)

set(eigen_examples
  ${CMAKE_CURRENT_SOURCE_DIR}/eigen_matrix.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/eigen_vector.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/eigen_map.cpp
)

set(hl_hdf5_examples
  ${CMAKE_CURRENT_SOURCE_DIR}/hl_hdf5_inmemory_files.cpp
)

set(parallel_hdf5_examples
  ${CMAKE_CURRENT_SOURCE_DIR}/parallel_hdf5_collective_io.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/parallel_hdf5_independent_io.cpp
)

set(half_float_examples
  ${CMAKE_CURRENT_SOURCE_DIR}/create_dataset_half_float.cpp
)

function(compile_example example_source)
  get_filename_component(example_filename ${example_source} NAME)
  string(REPLACE ".cpp" "_bin" example_name ${example_filename})

  add_executable(${example_name} ${example_source})
  target_link_libraries(${example_name} PUBLIC HighFive HighFiveWarnings)
  if(${ARGC} GREATER_EQUAL 2)
    target_link_libraries(${example_name} PUBLIC ${ARGV1} ${ARGV2} ${ARGV3} ${ARGV4})
  endif()
endfunction()


foreach(example_source ${core_examples})
  compile_example(${example_source} HighFiveFlags)
endforeach()

foreach(example_source ${easy_examples})
  compile_example(${example_source} HighFiveFlags HighFiveOptionalDependencies)
endforeach()

if(HIGHFIVE_TEST_SPAN)
  foreach(example_source ${span_examples})
    compile_example(${example_source} HighFiveFlags)
  endforeach()
endif()

if(HIGHFIVE_TEST_BOOST)
  foreach(example_source ${boost_examples})
    compile_example(${example_source} HighFiveFlags HighFiveBoostDependency)
  endforeach()
endif()

if(HIGHFIVE_TEST_EIGEN)
  foreach(example_source ${eigen_examples})
    compile_example(${example_source} HighFiveFlags HighFiveEigenDependency)
  endforeach()
endif()

if(HDF5_IS_PARALLEL)
  foreach(example_source ${parallel_hdf5_examples})
    compile_example(${example_source})
  endforeach()
endif()

add_library(HighFiveHlHdf5Dependency INTERFACE)
find_package(HDF5 QUIET COMPONENTS HL NAMES HDF5_HL)
if(${HDF5_HL_FOUND})
    message("HDF5 HL: ${HDF5_HL_LIBRARIES}")
    target_link_libraries(HighFiveHlHdf5Dependency ${HDF5_HL_LIBRARIES})

    foreach(example_source ${hl_hdf5_examples})
      compile_examples(${example_source} HighFiveFlags HighFiveHlHdf5Dependency)
    endforeach()
endif()

# TODO Half-float examples
