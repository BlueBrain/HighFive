#!/bin/sh
set -xeo pipefail
cd "$( dirname "${BASH_SOURCE[0]}")"  # cd here

BUILD_DIR="${PWD}/build-highfive"
ROOT="${PWD}/.."
TEST_DIR="${PWD}"
INSTALL_DIR="${BUILD_DIR}/install"

test_install() {
    local project="${1}"
    local project_dir="${TEST_DIR}/${project}"
    local dep_dir="${TEST_DIR}/${project}/deps/HighFive"
    shift

    pushd "${project_dir}"

    local build_dir="build"

    ln -sf ../../.. "${dep_dir}"

    cmake "$@" -B "${build_dir}" .
    cmake --build "${build_dir}" --verbose
    ctest --test-dir "${build_dir}"

    rm -f "${dep_dir}"
    rm -rf "${build_dir}"

    popd
}

cmake "${ROOT}" \
    -DHIGHFIVE_EXAMPLES=OFF \
    -DHIGHFIVE_UNIT_TESTS=OFF \
    -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
    -B "${BUILD_DIR}"
cmake --build "${BUILD_DIR}" --target install

for project in test_project test_dependent_library; do
    # Case 1. Base case: include subdirectory
    test_install "${project}"

    # Case 2. We use an install dir and all deps configuration
    # Install highfive (no tests required)
    test_install "${project}" \
        -DUSE_BUNDLED_HIGHFIVE=NO \
        -DHIGHFIVE_USE_INSTALL_DEPS=YES \
        -DCMAKE_PREFIX_PATH="${INSTALL_DIR}"

    # Case 3. We redetect-dependencies
    test_install "${project}" \
        -DUSE_BUNDLED_HIGHFIVE=NO \
        -DHIGHFIVE_USE_INSTALL_DEPS=NO \
        -DCMAKE_PREFIX_PATH="${INSTALL_DIR}"
done

rm -rf "${BUILD_DIR}"
