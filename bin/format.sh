#!/usr/bin/env bash
set -euo pipefail

clang_format_version="19.1.3"

script_dir="$(dirname "${BASH_SOURCE[0]}")"
venv_dir="$script_dir/../.clang-format-venv"

if [ ! -d "$venv_dir" ]; then
    python3 -m venv "$venv_dir"
    source "$venv_dir/bin/activate"
    pip install clang-format=="$clang_format_version"
    deactivate
fi

source "$venv_dir/bin/activate"

# Check if the installed version matches the expected version
installed_version="$(pip show clang-format | grep Version | cut -d ' ' -f 2)"

if [ "$installed_version" != "$clang_format_version" ]; then
    echo "Error: clang-format version mismatch. Expected $clang_format_version, got $installed_version"
    echo "Please remove the virtual environment and run the script again:"
    echo "    rm -r \"$venv_dir\" && \"$0\""
    exit 1
fi

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
    echo ""
    echo "To correct the formatting run:"
    echo "  $0"
    exit 1
fi
