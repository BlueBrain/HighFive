include_directories(${PROJECT_SOURCE_DIR}/include ${HDF5_INCLUDE_DIRS})

FILE(GLOB headers_highfive "${PROJECT_SOURCE_DIR}/include/highfive/*.hpp")
FILE(GLOB headers_highfive_bits "${PROJECT_SOURCE_DIR}/include/highfive/bits/*.hpp")


#[[function(compile_example exemple_source)

    get_filename_component(example_filename ${exemple_source} NAME)
    string(REPLACE ".cpp" "_bin" example_name ${example_filename})

    set(require_cpp11 FALSE)
    if(${example_filename} MATCHES ".*cpp11.*")
        set(require_cpp11 TRUE)
        if(NOT MSVC)
            add_definitions(-std=c++11)
        endif()
    endif()

    if(${example_filename} MATCHES ".*boost.*")
        if(NOT USE_BOOST)
            return()
        endif()
    endif()

    if(${example_filename} MATCHES ".*parallel_hdf5.*")
        if(NOT HIGHFIVE_PARALLEL_HDF5)
            return()
        endif()
    endif()

    if(require_cpp11)
        if(NOT COMPILER_SUPPORTS_CXX11)
            return()
	    endif()
    endif()

    add_executable(${example_name} ${exemple_source} ${headers_highfive} ${headers_highfive_bits})
    target_link_libraries(${example_name} ${HDF5_C_LIBRARIES})
    if(HIGHFIVE_PARALLEL_HDF5)
        target_link_libraries(${example_name} ${MPI_C_LIBRARIES})
    endif()

endfunction()


file(GLOB list_example "*.cpp")

foreach(example_src ${list_example})
compile_example(${example_src})

endforeach()]]
add_executable(select_Hmat_bin select_Hmat.cpp)
target_link_libraries(select_Hmat_bin HighFive)
target_compile_features(select_Hmat_bin PUBLIC cxx_auto_type)


add_executable(LDshrink_ex LDshrink_ex.cpp)

if (USE_BLOSC)
    add_definitions(-DUSE_BLOSC)
    target_link_libraries(LDshrink_ex blosc_filter)
endif ()
find_package(TBB REQUIRED)
set(BLA_VENDOR Intel10_64lp_seq)
find_package(BLAS)
find_package(LAPACK)

#find_package(MKL REQUIRED)
message("BLAS_LIBS='${BLAS_LIBRARIES}'")
message("HDF5_IS_PARALLEL='${HDF5_IS_PARALLEL}'")
target_link_libraries(LDshrink_ex tbb ${HDF5_C_LIBRARIES} cxxopts
        ${BLAS_LIBRARIES})

include_directories(${MKL_INCLUDE_DIRS})
target_compile_definitions(LDshrink_ex PUBLIC "-DEIGEN_USE_MKL_ALL")

