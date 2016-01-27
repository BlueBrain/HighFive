#
# Basic check for C++11 compiler support
#
#

include(CheckCXXCompilerFlag)

if(NOT DEFINED CXX11_INCOMPATIBLE_COMPILER)

if(CMAKE_CXX_COMPILER_ID STREQUAL "XL")
	set(CXX11_INCOMPATIBLE_COMPILER TRUE)
else()
	set(CXX11_INCOMPATIBLE_COMPILER FALSE)
endif()

endif()

if(NOT CXX11_INCOMPATIBLE_COMPILER)
	CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
endif()

if(COMPILER_SUPPORTS_CXX11)
	message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has C++11 support")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} does not have C++11 support")
endif()

