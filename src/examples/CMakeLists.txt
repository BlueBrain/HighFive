
function(compile_example exemple_source)

    get_filename_component(example_filename ${exemple_source} NAME)
    string(REPLACE ".cpp" "_bin" example_name ${example_filename})

    if(${example_filename} MATCHES ".*boost.*")
        if(Boost_FOUND)
            add_executable(${example_name} ${exemple_source})
            target_link_libraries(${example_name}
                HighFive
                Boost::boost
                Boost::system
                Boost::serialization)
        endif()
        return()
    endif()

    if(${example_filename} MATCHES ".*parallel_hdf5.*")
        if(NOT HIGHFIVE_PARALLEL_HDF5)
            return()
        endif()
    endif()

    add_executable(${example_name} ${exemple_source})
    target_link_libraries(${example_name} HighFive)

endfunction()


file(GLOB list_example "*.cpp")


foreach(example_src ${list_example})

    compile_example(${example_src})

endforeach()
