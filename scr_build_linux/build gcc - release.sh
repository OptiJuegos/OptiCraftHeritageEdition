#!/usr/bin/env bash
set -Eeuo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/_build_common.sh"
BUILD="$ROOT/build/gcc-release-linux"
[[ "${1:-}" == "clean" ]] && rm -rf "$BUILD"
build_native "$BUILD" -DCMAKE_BUILD_TYPE=Release -DMC_LOG_LEVEL=2
