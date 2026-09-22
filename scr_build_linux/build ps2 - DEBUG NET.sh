#!/usr/bin/env bash
set -Eeuo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/_build_common.sh"
BUILD="$ROOT/build/ps2-remote-debug"
ARG="${1:-}"
[[ "${ARG,,}" == "clean" ]] && rm -rf "$BUILD"
ARGS=(-DCMAKE_BUILD_TYPE=RelWithDebInfo -DPS2_REMOTE_DEBUG=ON -DPS2_ENABLE_NETWORK=ON -DPS2_ENABLE_SOUND=OFF -DPS2_ENABLE_PERSPECTIVE_TEXTURES=ON -DPS2_RENDER_STATS=OFF -DPS2_ENABLE_VU1_TERRAIN=ON -DMC_LOG_LEVEL=3)
if [[ "${ARG,,}" == "data" ]]; then stage_ps2_data "$BUILD" "${ARGS[@]}"; else build_ps2_profile "$BUILD" "${ARGS[@]}"; fi
