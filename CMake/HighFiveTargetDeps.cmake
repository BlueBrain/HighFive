# Link against target system libs
# -------------------------------

if(NOT TARGET libdeps)

  # Independent target to make it possible to have new dependencies each build
  add_library(libdeps INTERFACE)

  if(HIGHFIVE_VERBOSE)
    target_compile_definitions(libdeps INTERFACE -DHIGHFIVE_LOG_LEVEL=0)
  endif()

  if(HIGHFIVE_GLIBCXX_ASSERTIONS)
    target_compile_definitions(libdeps INTERFACE -D_GLIBCXX_ASSERTIONS)
  endif()

  if(HIGHFIVE_HAS_FRIEND_DECLARATIONS)
    target_compile_definitions(libdeps INTERFACE -DHIGHFIVE_HAS_FRIEND_DECLARATIONS=1)
  endif()

  if(HIGHFIVE_SANITIZER)
    target_compile_options(libdeps INTERFACE -fsanitize=${HIGHFIVE_SANITIZER})
    target_link_options(libdeps INTERFACE -fsanitize=${HIGHFIVE_SANITIZER})
  endif()

  # HDF5
  if(NOT DEFINED HDF5_C_LIBRARIES)
    set(HDF5_PREFER_PARALLEL ${HIGHFIVE_PARALLEL_HDF5})
    set(HDF5_USE_STATIC_LIBRARIES ${HIGHFIVE_STATIC_HDF5})
    find_package(HDF5 REQUIRED)
  endif()

  if(HIGHFIVE_PARALLEL_HDF5 AND NOT HDF5_IS_PARALLEL)
    message(WARNING "Parallel HDF5 requested but libhdf5 doesnt support it")
  endif()

  target_include_directories(libdeps SYSTEM INTERFACE ${HDF5_INCLUDE_DIRS})
  target_link_libraries(libdeps INTERFACE ${HDF5_LIBRARIES})
  target_compile_definitions(libdeps INTERFACE ${HDF5_DEFINITIONS})
  target_compile_definitions(libdeps INTERFACE HIGHFIVE_HAS_CONCEPTS=$<BOOL:${HIGHFIVE_HAS_CONCEPTS}>)


  # Boost
  if(HIGHFIVE_USE_BOOST)
    if(NOT DEFINED Boost_NO_BOOST_CMAKE)
      # HighFive deactivated finding Boost via Boost's own CMake files
      # in Oct 2016 (commit '25627b085'). Likely to appease one cluster.
      # Boost's CMake support has since improved and likely this setting
      # isn't needed anymore. It is kept for backwards compatibility.
      # However, a rework of HighFive's CMake code should consider removing
      # this default. Hard coding this to true has been reported to cause
      # build failures.
      set(Boost_NO_BOOST_CMAKE TRUE)
    endif()
    find_package(Boost REQUIRED COMPONENTS system serialization)
    # Dont use imported targets yet, not avail before cmake 3.5
    target_include_directories(libdeps SYSTEM INTERFACE ${Boost_INCLUDE_DIR})
    target_compile_definitions(libdeps INTERFACE BOOST_ALL_NO_LIB H5_USE_BOOST)
  endif()

  # Half
  if(HIGHFIVE_USE_HALF_FLOAT)
    find_file(FOUND_HALF half.hpp)
    if (NOT FOUND_HALF)
      message(FATAL_ERROR "Half-precision floating-point support requested but file half.hpp not found")
    endif()
    target_compile_definitions(libdeps INTERFACE H5_USE_HALF_FLOAT)
  endif()

  # Eigen
  if(HIGHFIVE_USE_EIGEN)
    if (NOT EIGEN3_INCLUDE_DIRS)
      find_package(Eigen3 NO_MODULE)
      if(Eigen3_FOUND)
        message(STATUS "Found Eigen ${Eigen3_VERSION}: ${EIGEN3_INCLUDE_DIRS}")
      else()
        find_package(PkgConfig)
        pkg_check_modules(EIGEN3 REQUIRED eigen3)
      endif()
    endif()
    if (NOT EIGEN3_INCLUDE_DIRS)
      message(FATAL_ERROR "Eigen was requested but could not be found")
    endif()
    target_include_directories(libdeps SYSTEM INTERFACE ${EIGEN3_INCLUDE_DIRS})
    target_compile_definitions(libdeps INTERFACE H5_USE_EIGEN)
  endif()

  # xtensor
  if(HIGHFIVE_USE_XTENSOR)
    if (NOT xtensor_INCLUDE_DIRS)
      find_package(xtensor REQUIRED)
    endif()
    if (NOT xtl_INCLUDE_DIRS)
      find_package(xtl REQUIRED)
    endif()
    target_include_directories(libdeps SYSTEM INTERFACE ${xtensor_INCLUDE_DIRS} ${xtl_INCLUDE_DIRS})
    target_compile_definitions(libdeps INTERFACE H5_USE_XTENSOR)
  endif()

  # OpenCV
  if(HIGHFIVE_USE_OPENCV)
    if (NOT OpenCV_INCLUDE_DIRS)
      find_package(OpenCV REQUIRED)
    endif()
    target_include_directories(libdeps SYSTEM INTERFACE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(libdeps INTERFACE ${OpenCV_LIBS})
    target_compile_definitions(libdeps INTERFACE H5_USE_OPENCV)
  endif()

  # MPI
  if(HIGHFIVE_PARALLEL_HDF5 OR HDF5_IS_PARALLEL)
    find_package(MPI REQUIRED)
    target_include_directories(libdeps SYSTEM INTERFACE ${MPI_CXX_INCLUDE_PATH})
    target_link_libraries(libdeps INTERFACE ${MPI_CXX_LIBRARIES})
    if(CMAKE_VERSION VERSION_LESS 3.13)
      target_link_libraries(libdeps INTERFACE ${MPI_CXX_LINK_FLAGS})
    else()
      target_link_options(libdeps INTERFACE "SHELL:${MPI_CXX_LINK_FLAGS}")
    endif()
  endif()

endif()
