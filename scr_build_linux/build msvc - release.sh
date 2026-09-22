#!/usr/bin/env bash
set -Eeuo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/_build_common.sh"
echo "NOTE: MSVC is Windows-only. This Linux equivalent performs a native Release build using the system C/C++ compiler."
BUILD="$ROOT/build/native-release-linux"
[[ "${1:-}" == "clean" ]] && rm -rf "$BUILD"
build_native "$BUILD" -DCMAKE_BUILD_TYPE=Release
