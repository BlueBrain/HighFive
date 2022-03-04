#! /usr/bin/env bash

DOXYGEN_AWESOME_DIR="$(dirname "$(realpath "$0")")"

REPO_URL="https://raw.githubusercontent.com/jothepro/doxygen-awesome-css"
VERSION="v2.0.1"
STYLESHEET="doxygen-awesome.css"

curl "${REPO_URL}/${VERSION}/${STYLESHEET}" \
     --output "${DOXYGEN_AWESOME_DIR}/${STYLESHEET}"
