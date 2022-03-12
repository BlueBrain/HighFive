#! /usr/bin/env bash

#
# Copyright (c), 2022, Blue Brain Project
#
# Distributed under the Boost Software License, Version 1.0.
#   (See accompanying file LICENSE_1_0.txt or copy at
#         http://www.boost.org/LICENSE_1_0.txt)
#

set -e

if [[ $# -ne 1 ]]
then
  echo "Usage: $0 TMP_DIR"
  echo ""
  echo "TMP_DIR must point to a writeable, empty, temporary directory."

  exit 1
fi

TMP_DIR="$(realpath "$1")"
DOXYGEN_AWESOME_DIR="$(dirname "$(realpath "$0")")"
REPO_URL="https://github.com/jothepro/doxygen-awesome-css"
REPO_DIR="${TMP_DIR}/doxygen-awesome-css"

CONTENT_URL="https://raw.githubusercontent.com/jothepro/doxygen-awesome-css"

mkdir -p "${TMP_DIR}"
git clone ${REPO_URL} "${REPO_DIR}" 1>&2
pushd "${REPO_DIR}" 1>&2

VERSION="$(git tag -l | sed -e '/^v[0-9]*\.[0-9]*\.[0-9]*$/!d' | sort -V | tail -n 1)"

popd 1>&2

if [[ -z "$VERSION" ]]
then
  exit 1
fi

STYLESHEET="doxygen-awesome.css"
curl "${CONTENT_URL}/${VERSION}/${STYLESHEET}" \
     --output "${DOXYGEN_AWESOME_DIR}/${STYLESHEET}" \
     1>&2

echo "${VERSION}"
