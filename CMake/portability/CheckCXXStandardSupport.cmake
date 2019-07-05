#
# Basic check for C++ compiler startndard support
#

include(CheckCXXCompilerFlag)

if (CMAKE_CXX_COMPILER_ID MATCHES "XL")
    set(COMPILER_SUPPORTS_CXX11 FALSE)
    return()
endif()

check_cxx_compiler_flag("${CMAKE_CXX11_STANDARD_COMPILE_OPTION}" COMPILER_SUPPORTS_CXX11)
check_cxx_compiler_flag("${CMAKE_CXX14_STANDARD_COMPILE_OPTION}" COMPILER_SUPPORTS_CXX14)
check_cxx_compiler_flag("${CMAKE_CXX17_STANDARD_COMPILE_OPTION}" COMPILER_SUPPORTS_CXX17)
