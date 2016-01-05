
if(UNIT_TESTS AND USE_BOOST)


include_directories(${CMAKE_SOURCE_DIR}/include ${HDF5_INCLUDE_DIRS})

# silent boost warnings
add_definitions(-Wno-unused-parameter -Wno-unused-local-typedef)

file(GLOB tests_high_five_src "*.cpp")

add_executable(tests_high_five_bin ${tests_high_five_src})
target_link_libraries(tests_high_five_bin ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} ${HDF5_C_LIBRARIES} ${HDF5_HL_LIBRARIES})

add_test(NAME tests_high_five COMMAND tests_high_five_bin)


endif()
