#! /usr/bin/env bash

set -e

if [[ $# -eq 0 ]]
then
  examples_dir="."
elif [[ $# -eq 1 ]]
then
  examples_dir="$1"
else
  echo "Usage: $0 [EXAMPLES_DIR]"
  exit -1
fi

for f in "${examples_dir}"/*_bin
do
  echo "-- ${f}"
  "${f}"
done
