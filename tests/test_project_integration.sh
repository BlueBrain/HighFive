#!/bin/sh
set -xeo pipefail
cd "$( dirname "${BASH_SOURCE[0]}")"  # cd here

rm -rf _highfive_build
mkdir -p _highfive_build
pushd _highfive_build
cmake ../.. -DHIGHFIVE_EXAMPLES=OFF -DHIGHFIVE_UNIT_TESTS=OFF -DCMAKE_INSTALL_PREFIX=$PWD/install
make install
popd

for project in test_project test_library; do
    # Case 1. Base case: include subdirectory
    rm -rf ${project}/_build_subdir
    mkdir -p ${project}/_build_subdir
    pushd ${project}/_build_subdir
    cmake ..
    make VERBOSE=1
    ctest
    popd

    # Case 2. We use an install dir and all deps configuration
    # Install highfive (no tests required)

    rm -rf ${project}/_build_install_reuse_deps
    mkdir -p ${project}/_build_install_reuse_deps
    pushd ${project}/_build_install_reuse_deps
    cmake .. -DUSE_BUNDLED_HIGHFIVE=NO \
        -DHIGHFIVE_USE_INSTALL_DEPS=YES \
        -DCMAKE_PREFIX_PATH=$PWD/../../_highfive_build/install
    make VERBOSE=1
    ctest
    popd

    # Case 3. We redetect-dependencies
    rm -rf ${project}/_build_install_new_deps
    mkdir -p ${project}/_build_install_new_deps
    pushd ${project}/_build_install_new_deps
    cmake .. -DUSE_BUNDLED_HIGHFIVE=NO \
        -DHIGHFIVE_USE_INSTALL_DEPS=NO \
        -DCMAKE_PREFIX_PATH=$PWD/../../_highfive_build/install
    make VERBOSE=1
    ctest
    popd
done
