

set(H5_DATASET_SRC "createdataset.cpp")

include_directories(${CMAKE_SOURCE_DIR}/include ${HDF5_INCLUDE_DIRS})

FILE(GLOB headers_highfive "${CMAKE_SOURCE_DIR}/include/highfive/*.hpp")
FILE(GLOB headers_highfive_bits "${CMAKE_SOURCE_DIR}/include/highfive/bits/*.hpp")

add_executable(h5-create-dataset ${H5_DATASET_SRC} ${headers_highfive} ${headers_highfive_bits})
target_link_libraries(h5-create-dataset ${HDF5_LIBRARIES})
