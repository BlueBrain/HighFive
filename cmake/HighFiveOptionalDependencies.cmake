if(HIGHFIVE_TEST_BOOST AND NOT TARGET HighFiveBoostDependency)
  add_library(HighFiveBoostDependency INTERFACE)
  find_package(Boost REQUIRED)
  target_link_libraries(HighFiveBoostDependency INTERFACE Boost::headers)
  # TODO check if we need Boost::disable_autolinking to cause:
  # -DBOOST_ALL_NO_LIB (does something on MSVC).
endif()

if(HIGHFIVE_TEST_EIGEN AND NOT TARGET HighFiveEigenDependency)
  add_library(HighFiveEigenDependency INTERFACE)
  find_package(Eigen3 REQUIRED NO_MODULE)
  target_link_libraries(HighFiveEigenDependency INTERFACE Eigen3::Eigen)
endif()

if(HIGHFIVE_TEST_XTENSOR AND NOT TARGET HighFiveXTensorDependency)
  add_library(HighFiveXTensorDependency INTERFACE)
  find_package(xtensor REQUIRED)
  target_link_libraries(HighFiveXTensorDependency INTERFACE xtensor)
endif()

if(HIGHFIVE_TEST_OPENCV AND NOT TARGET HighFiveOpenCVDependency)
  add_library(HighFiveOpenCVDependency INTERFACE)
  find_package(OpenCV REQUIRED)
  target_include_directories(HighFiveOpenCVDependency SYSTEM INTERFACE ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(HighFiveOpenCVDependency INTERFACE ${OpenCV_LIBS})
  target_compile_definitions(HighFiveOpenCVDependency INTERFACE H5_USE_OPENCV)
endif()
