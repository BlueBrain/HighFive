if(TARGET HighFiveWarnings)
    # Allow multiple `include(HighFiveWarnings)`, which would
    # attempt to redefine `HighFiveWarnings` and fail without
    # this check.
    return()
endif()

add_library(HighFiveWarnings INTERFACE)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang"
   OR CMAKE_CXX_COMPILER_ID MATCHES "GNU"
   OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")

    target_compile_options(HighFiveWarnings
        INTERFACE
            -Wall
            -Wextra
            -Wshadow
            -Wnon-virtual-dtor
            -Wunused
            -Woverloaded-virtual
            -Wformat=2
            -Wconversion
            -Wsign-conversion
            -Wno-error=deprecated-declarations
    )

    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Intel")
        target_compile_options(HighFiveWarnings
            INTERFACE
                -Wpedantic
                -Wcast-align
                -Wdouble-promotion
        )
    endif()
endif()
