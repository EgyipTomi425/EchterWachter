#!/usr/bin/env bash
set -euo pipefail
repo_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
export CXX="${CXX:-clang++}"
systemctl stop echterwachter.service
if [[ $(systemctl show echterwachter.service --property=ActiveState --value) != inactive ]]; then
    echo 'The service must be inactive before building.' >&2
    exit 1
fi
# Public dependencies follow master. An absent private SDK is intentionally skipped.
git -C "$repo_dir" -c fetch.recurseSubmodules=false submodule update --init --recursive --remote
sdk_options=()
if [[ -f "$repo_dir/src/custom/gge_sdk/CMakeLists.txt" ]]; then
    if GIT_TERMINAL_PROMPT=0 git -C "$repo_dir/src/custom" \
            -c core.sshCommand='ssh -o BatchMode=yes' \
            submodule update --remote --checkout gge_sdk; then
        git -C "$repo_dir/src/custom/gge_sdk" submodule update --init --recursive --remote
    else
        echo 'Private SDK update unavailable; building without the SDK.'
        sdk_options=(-DGGE_SDK=OFF)
    fi
fi
# Leave it stopped on failure; only start the newly built binary after checks pass.
cmake -S "$repo_dir" -B "$repo_dir/build" -G Ninja "$@" "${sdk_options[@]}"
cmake --build "$repo_dir/build" --parallel 4
ctest --test-dir "$repo_dir/build" --output-on-failure --no-tests=error
systemctl start echterwachter.service
