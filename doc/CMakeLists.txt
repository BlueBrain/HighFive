if(HIGH_FIVE_DOCUMENTATION)
find_package(Pandoc REQUIRED)

configure_file(build.sh.cmake build.sh @ONLY)

add_custom_target(doc ALL DEPENDS
	COMMAND sh ${CMAKE_CURRENT_BINARY_DIR}/build.sh
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/build.sh
)

install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/index.html
    DESTINATION ${HTML_INSTALL_DIR}
)
install(
	DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src
    DESTINATION ${HTML_INSTALL_DIR}
)

find_package(Doxygen)
if(Doxygen_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doxygen ALL COMMAND Doxygen::doxygen ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
else()
    message(STATUS "Doxygen documentation will not be build because Doxygen has not been found.")
endif()
endif(HIGH_FIVE_DOCUMENTATION)
