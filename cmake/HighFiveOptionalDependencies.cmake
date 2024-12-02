if(NOT TARGET HighFiveBoostDependency)
  add_library(HighFiveBoostDependency INTERFACE)
  if(HIGHFIVE_TEST_BOOST)
    find_package(Boost REQUIRED)
    target_link_libraries(HighFiveBoostDependency INTERFACE Boost::headers)
    # TODO check if we need Boost::disable_autolinking to cause:
    # -DBOOST_ALL_NO_LIB (does something on MSVC).
    target_compile_definitions(HighFiveBoostDependency INTERFACE HIGHFIVE_TEST_BOOST=1)
  endif()
  if(HIGHFIVE_TEST_BOOST_SPAN)
    target_compile_definitions(HighFiveBoostDependency INTERFACE HIGHFIVE_TEST_BOOST_SPAN=1)
  endif()
endif()

if(NOT TARGET HighFiveEigenDependency)
  add_library(HighFiveEigenDependency INTERFACE)
  if(HIGHFIVE_TEST_EIGEN)
    find_package(Eigen3 REQUIRED NO_MODULE)
    target_link_libraries(HighFiveEigenDependency INTERFACE Eigen3::Eigen)
    target_compile_definitions(HighFiveEigenDependency INTERFACE HIGHFIVE_TEST_EIGEN=1)
  endif()
endif()

if(NOT TARGET HighFiveXTensorDependency)
  add_library(HighFiveXTensorDependency INTERFACE)
  if(HIGHFIVE_TEST_XTENSOR)
    find_package(xtensor REQUIRED)
    target_link_libraries(HighFiveXTensorDependency INTERFACE xtensor)
    target_compile_definitions(HighFiveXTensorDependency INTERFACE HIGHFIVE_TEST_XTENSOR=1)
  endif()
endif()

if(NOT TARGET HighFiveOpenCVDependency)
  add_library(HighFiveOpenCVDependency INTERFACE)
  if(HIGHFIVE_TEST_OPENCV)
    find_package(OpenCV REQUIRED)
    target_include_directories(HighFiveOpenCVDependency SYSTEM INTERFACE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(HighFiveOpenCVDependency INTERFACE ${OpenCV_LIBS})
    target_compile_definitions(HighFiveOpenCVDependency INTERFACE HIGHFIVE_TEST_OPENCV=1)
  endif()
endif()

if(NOT TARGET HighFiveSpanDependency)
  add_library(HighFiveSpanDependency INTERFACE)
  if(HIGHFIVE_TEST_SPAN)
    target_compile_definitions(HighFiveSpanDependency INTERFACE HIGHFIVE_TEST_SPAN=1)
  endif()
endif()

if(NOT TARGET HighFiveOptionalDependencies)
  add_library(HighFiveOptionalDependencies INTERFACE)
  target_link_libraries(HighFiveOptionalDependencies INTERFACE
    HighFiveBoostDependency
    HighFiveEigenDependency
    HighFiveXTensorDependency
    HighFiveOpenCVDependency
    HighFiveSpanDependency
  )
endif()
