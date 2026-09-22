#!/usr/bin/env bash
set -Eeuo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/_build_common.sh"
BUILD="$ROOT/build/ps2-release"
[[ "${1:-}" == "clean" ]] && rm -rf "$BUILD"
build_ps2_profile "$BUILD" -DCMAKE_BUILD_TYPE=Release -DPS2_ENABLE_NETWORK=ON -DPS2_ENABLE_SOUND=ON -DPS2_ENABLE_PERSPECTIVE_TEXTURES=ON -DPS2_RENDER_STATS=ON -DPS2_ENABLE_VU1_TERRAIN=OFF -DMC_LOG_LEVEL=3
