#!/bin/sh
set -xe
cd "$( dirname "${BASH_SOURCE[0]}")"  # cd here

# Case 1. Base case: include subdirectory
mkdir -p test_project/_build_subdir
pushd test_project/_build_subdir
cmake .. && make VERBOSE=1 && ctest
popd

# Case 2. We use an install dir and all deps configuration
# Install highfive (no tests required)
mkdir -p _highfive_build
pushd _highfive_build
cmake ../.. -DHIGHFIVE_EXAMPLES=OFF -DHIGHFIVE_UNIT_TESTS=OFF -DCMAKE_INSTALL_PREFIX=$PWD/install
make install
popd

mkdir -p test_project/_build_install_reuse_deps
pushd test_project/_build_install_reuse_deps
cmake .. -DUSE_BUNDLED_HIGHFIVE=NO \
    -DHIGHFIVE_USE_INSTALL_DEPS=YES \
    -DCMAKE_PREFIX_PATH=$PWD/../../_highfive_build/install
make VERBOSE=1 && ctest
popd

# Case 3. We redetect-dependencies
mkdir -p test_project/_build_install_new_deps
pushd test_project/_build_install_new_deps
cmake .. -DUSE_BUNDLED_HIGHFIVE=NO \
    -DHIGHFIVE_USE_INSTALL_DEPS=NO \
    -DCMAKE_PREFIX_PATH=$PWD/../../_highfive_build/install
make VERBOSE=1 && ctest
popd
