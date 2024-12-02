if(TARGET HighFiveFlags)
    # Allow multiple `include(HighFiveWarnings)`, which would
    # attempt to redefine `HighFiveWarnings` and fail without
    # this check.
    return()
endif()

add_library(HighFiveFlags INTERFACE)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    if(HIGHFIVE_MAX_ERRORS)
        target_compile_options(HighFiveFlags
            INTERFACE
            -fmax-errors=${HIGHFIVE_MAX_ERRORS}
        )
    endif()
endif()

if(HIGHFIVE_GLIBCXX_ASSERTIONS)
    target_compile_definitions(HighFiveFlags INTERFACE -D_GLIBCXX_ASSERTIONS)
endif()

if(HIGHFIVE_HAS_FRIEND_DECLARATIONS)
    target_compile_definitions(HighFiveFlags INTERFACE -DHIGHFIVE_HAS_FRIEND_DECLARATIONS=1)
endif()

if(HIGHFIVE_SANITIZER)
    target_compile_options(HighFiveFlags INTERFACE -fsanitize=${HIGHFIVE_SANITIZER})
    target_link_options(HighFiveFlags INTERFACE -fsanitize=${HIGHFIVE_SANITIZER})
endif()
