#!/bin/sh
set -xeo pipefail
cd "$( dirname "${BASH_SOURCE[0]}")"  # cd here

BUILDDIR="${PWD}/build"
ROOT="${PWD}/.."
TESTDIR="${PWD}"

test_install() {
    local project="${1}"
    local builddir="${BUILDDIR}/${project}/${2}"
    shift
    shift
    ln -s ../../.. "${TESTDIR}/${project}/deps/HighFive"
    rm -rf "${builddir}"
    mkdir -p "${builddir}"
    pushd "${builddir}"
    cmake "${TESTDIR}/${project}" "$@"
    make VERBOSE=1
    ctest
    popd
    rm "${TESTDIR}/${project}/deps/HighFive"
}

rm -rf "${BUILDDIR}/highfive"
mkdir -p "${BUILDDIR}/highfive"
pushd "${BUILDDIR}/highfive"
cmake "${ROOT}" \
    -DHIGHFIVE_EXAMPLES=OFF \
    -DHIGHFIVE_UNIT_TESTS=OFF \
    -DCMAKE_INSTALL_PREFIX="${PWD}/install"
make install
popd

for project in test_project test_dependent_library; do
    # Case 1. Base case: include subdirectory
    test_install "${project}" subdir

    # Case 2. We use an install dir and all deps configuration
    # Install highfive (no tests required)
    test_install "${project}" reuse_deps \
        -DUSE_BUNDLED_HIGHFIVE=NO \
        -DHIGHFIVE_USE_INSTALL_DEPS=YES \
        -DCMAKE_PREFIX_PATH="${BUILDDIR}/highfive/install"
            #
    # Case 3. We redetect-dependencies
    test_install "${project}" install_new_deps \
        -DUSE_BUNDLED_HIGHFIVE=NO \
        -DHIGHFIVE_USE_INSTALL_DEPS=NO \
        -DCMAKE_PREFIX_PATH="${BUILDDIR}/highfive/install"
done
