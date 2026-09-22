#!/usr/bin/env bash
set -Eeuo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/_build_common.sh"
ensure_32bit_toolchain
BUILD="$ROOT/build/gcc32-legacy-release-linux"
[[ "${1:-}" == "clean" ]] && rm -rf "$BUILD"
build_native "$BUILD" -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ \
  -DCMAKE_C_FLAGS="-m32 -msse2 -mfpmath=sse" \
  -DCMAKE_CXX_FLAGS="-m32 -msse2 -mfpmath=sse" \
  -DCMAKE_EXE_LINKER_FLAGS="-m32" \
  -DCMAKE_SHARED_LINKER_FLAGS="-m32" \
  -DCMAKE_MODULE_LINKER_FLAGS="-m32" \
  -DPC_LEGACY_BUILD=OFF -DOPTICRAFT_ENABLE_LTO=OFF -DMC_LOG_LEVEL=0
