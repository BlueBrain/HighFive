include_directories(${HDF5_INCLUDE_DIRS})

function(compile_example exemple_source)

    get_filename_component(example_filename ${exemple_source} NAME)
    string(REPLACE ".cpp" "_bin" example_name ${example_filename})

    if(${example_filename} MATCHES ".*eigen.*")
        if(NOT HIGHFIVE_USE_EIGEN)
            return()
        endif()
    endif()

    if(${example_filename} MATCHES ".*boost.*")
        if(NOT HIGHFIVE_USE_BOOST)
            return()
        endif()
    endif()

    if(${example_filename} MATCHES ".*parallel_hdf5.*")
        if(NOT HIGHFIVE_PARALLEL_HDF5)
            return()
        endif()
    endif()

    if(${example_filename} MATCHES ".*half_float.*")
        if(NOT HIGHFIVE_USE_HALF_FLOAT)
            return()
        endif()
    endif()

    if(${example_name} MATCHES ".*hl_hdf5.*")
        find_package(HDF5 QUIET COMPONENTS HL NAMES HDF5_HL)
        if(${HDF5_FL_FOUND})
            add_executable(${example_name} ${exemple_source})
            target_link_libraries(${example_name} HighFive ${HDF5_HL_LIBRARIES})
        endif()
    else()
        add_executable(${example_name} ${exemple_source})
        target_link_libraries(${example_name} HighFive)
    endif()

endfunction()

file(GLOB list_example "*.cpp")

foreach(example_src ${list_example})
    compile_example(${example_src})
endforeach()
