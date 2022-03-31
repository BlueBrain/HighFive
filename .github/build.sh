# Build env

[ "$CC" ] && $CC --version
cmake --version
set -x
cmake -B $GITHUB_WORKSPACE/build -S $GITHUB_WORKSPACE \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
  -DHIGHFIVE_USE_EIGEN:BOOL=TRUE \
  "${CMAKE_OPTIONS[@]}"
cmake --build $GITHUB_WORKSPACE/build --config $BUILD_TYPE --parallel 2 --verbose
