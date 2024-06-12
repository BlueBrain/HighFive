# Build env

[ "$CC" ] && $CC --version
cmake --version
set -x
export HIGHFIVE_BUILD=$GITHUB_WORKSPACE/build
cmake -B $HIGHFIVE_BUILD -S $GITHUB_WORKSPACE \
  -DHIGHFIVE_HAS_WERROR=On \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
  "${CMAKE_OPTIONS[@]}"
cmake --build $HIGHFIVE_BUILD --config $BUILD_TYPE --parallel 2 --verbose
