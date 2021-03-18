# Build env

[ "$CC" ] && $CC --version
cmake --version
set -x
cmake -E make_directory $GITHUB_WORKSPACE/build
cd $GITHUB_WORKSPACE/build
cmake $GITHUB_WORKSPACE \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
  -DHIGHFIVE_USE_EIGEN:BOOL=TRUE \
  "${CMAKE_OPTIONS[@]}"


echo ">>> BUILDING"
cmake --build . --config $BUILD_TYPE --parallel 2 --verbose
