#
# Basic check for C++11 compiler support
#
#

include(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)

if(COMPILER_SUPPORTS_CXX11)
	message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has C++11 support")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} does not have C++11 support")
endif()

