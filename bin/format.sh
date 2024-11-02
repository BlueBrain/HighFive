#!/usr/bin/env bash
set -euo pipefail

script_dir="$(dirname "${BASH_SOURCE[0]}")"
venv_dir="$script_dir/../.clang-format-venv"

if [ ! -d "$venv_dir" ]; then
    python3 -m venv "$venv_dir"
    source "$venv_dir/bin/activate"
    pip install clang-format==19.1.3
    deactivate
fi

source "$venv_dir/bin/activate"

clang-format --version
for i in $(git ls-files | grep ".[ch]pp$"); do
    clang-format -i "$i" > /dev/null 2>&1
done

modified_files=$(git diff --name-only)
if [ -n "$modified_files" ]; then
    echo "Some files are not well formatted:"
    echo "$modified_files"
    echo ""
    echo "The diff is:"
    git --no-pager diff
    exit 1
fi
