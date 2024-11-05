#!/usr/bin/env bash
set -xeuo pipefail
cd "$( dirname "${BASH_SOURCE[0]}")"  # cd here

# All output should be within this directory.
TMP_DIR="${PWD}/tmp"

# Root of the cmake integration tests.
TEST_DIR="${PWD}"

# Path of the sources, build and install directory of HighFive.
HIGHFIVE_DIR="${TEST_DIR}/../.."
HIGHFIVE_BUILD_DIR="${TMP_DIR}/build-highfive"
HIGHFIVE_INSTALL_DIR="${HIGHFIVE_BUILD_DIR}/install"

export HIGHFIVE_GIT_REPOSITORY="file://$(realpath "$HIGHFIVE_DIR")"
export HIGHFIVE_GIT_TAG=$(git rev-parse HEAD)

make_submodule() {
    local project_dir="$1"
    local dep_dir="${project_dir}/deps/HighFive"

    rm "${dep_dir}" || true
    mkdir -p "$(dirname "${dep_dir}")"
    ln -sf "${HIGHFIVE_DIR}" "${dep_dir}"
}

test_dependent_library() {
    local project="dependent_library"
    local project_dir="${TEST_DIR}/${project}"

    for use_boost in On Off
    do
      local build_dir="${TMP_DIR}/build"
      local install_dir="${TMP_DIR}/build/install"

      rm -rf ${build_dir} || true

      cmake "$@" \
            -DUSE_BOOST=${use_boost} \
            -DCMAKE_PREFIX_PATH="${HIGHFIVE_INSTALL_DIR}" \
            -DCMAKE_INSTALL_PREFIX="${install_dir}" \
            -B "${build_dir}" "${project_dir}"

      cmake --build "${build_dir}" --verbose --target install


      for vendor in submodule fetch_content external none
      do
        local test_project="test_dependent_library"
        local test_build_dir="${TMP_DIR}/test_build"
        local test_install_dir="${TMP_DIR}/test_build/install"

        make_submodule ${test_project}


        rm -rf ${test_build_dir} || true

        cmake -DUSE_BOOST=${use_boost} \
              -DVENDOR_STRATEGY=${vendor} \
              -DCMAKE_PREFIX_PATH="${HIGHFIVE_INSTALL_DIR};${install_dir}" \
              -DCMAKE_INSTALL_PREFIX="${test_install_dir}" \
              -B "${test_build_dir}" "${test_project}"

        cmake --build "${test_build_dir}" --verbose
        ctest --test-dir "${test_build_dir}" --verbose
      done
    done
}

test_application() {
    local project="application"
    local project_dir="${TEST_DIR}/${project}"

    make_submodule ${project_dir}

    for vendor in submodule fetch_content external
    do
      for use_boost in On Off
      do
        local build_dir="${TMP_DIR}/build"
        local install_dir="${TMP_DIR}/build/install"

        rm -rf ${build_dir} || true

        cmake "$@" \
              -DUSE_BOOST=${use_boost} \
              -DVENDOR_STRATEGY=${vendor} \
              -DCMAKE_PREFIX_PATH="${HIGHFIVE_INSTALL_DIR}" \
              -DCMAKE_INSTALL_PREFIX="${install_dir}" \
              -B "${build_dir}" "${project_dir}"

        cmake --build "${build_dir}" --verbose --target install
        ctest --test-dir "${build_dir}"
        "${install_dir}"/bin/Hi5Application
      done
    done
}

cmake -DHIGHFIVE_EXAMPLES=OFF \
      -DHIGHFIVE_UNIT_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX="${HIGHFIVE_INSTALL_DIR}" \
      -B "${HIGHFIVE_BUILD_DIR}" \
      "${HIGHFIVE_DIR}"

cmake --build "${HIGHFIVE_BUILD_DIR}" --target install

for integration in Include full short bailout
do
  test_dependent_library \
      -DINTEGRATION_STRATEGY=${integration}

  test_application \
      -DINTEGRATION_STRATEGY=${integration}
done
