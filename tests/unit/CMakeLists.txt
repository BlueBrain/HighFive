

if(NOT HIGHFIVE_UNIT_TESTS)
  return()
endif()


find_package(Boost COMPONENTS system serialization unit_test_framework REQUIRED)


foreach(test_name tests_high_five_base tests_high_five_multi_dims tests_high_five_easy)

    add_executable(${test_name} "${test_name}.cpp")

    target_link_libraries(${test_name}
        HighFive
        Boost::boost
        Boost::system
        Boost::serialization
        Boost::unit_test_framework)

    add_test(NAME ${test_name} COMMAND ${test_name})

endforeach()


if(HIGHFIVE_PARALLEL_HDF5)

    file(GLOB tests_parallel_src "*parallel.cpp")

    find_package(MPI REQUIRED)

    add_executable(tests_parallel_bin ${tests_parallel_src})

    target_link_libraries(tests_parallel_bin
        ${MPI_CXX_LIBRARIES}
        HighFive
        Boost::boost
        Boost::system
        Boost::serialization
        Boost::unit_test_framework)

    target_compile_definitions(tests_parallel_bin "MPI_NO_CPPBIND")

    target_include_directories(tests_parallel_bin ${MPI_CXX_INCLUDE_PATH})

    add_test(NAME tests_parallel COMMAND ${TEST_MPI_EXEC_PREFIX_DEFAULT} -n 2
        ${CMAKE_CURRENT_BINARY_DIR}/tests_parallel_bin)

endif()


option(HIGHFIVE_TEST_SINGLE_INCLUDES "Enable testing single includes" FALSE)

if(HIGHFIVE_TEST_SINGLE_INCLUDES)

    file(
        GLOB public_headers
        LIST_DIRECTORIES false
        RELATIVE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/include/highfive/*.hpp)

    foreach(PUBLIC_HEADER ${public_headers})

        get_filename_component(CLASS_NAME ${PUBLIC_HEADER} NAME_WE)

        configure_file(tests_import_public_headers.cpp "tests_${CLASS_NAME}.cpp" @ONLY)

        add_executable("tests_include_${CLASS_NAME}" "${CMAKE_CURRENT_BINARY_DIR}/tests_${CLASS_NAME}.cpp")

        target_link_libraries("tests_include_${CLASS_NAME}"
            HighFive
            Boost::boost
            Boost::system
            Boost::serialization
            Boost::unit_test_framework)

    endforeach()

endif()
