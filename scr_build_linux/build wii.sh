#!/usr/bin/env bash
set -Eeuo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/_build_common.sh"

MODE="bringup"
CLEAN=0
DATA=0
EXTRA=()

for arg in "$@"; do
    case "${arg,,}" in
        clean) CLEAN=1 ;;
        game) MODE="game" ;;
        data) DATA=1 ;;
        *) EXTRA+=("$arg") ;;
    esac
done

if [[ "$MODE" == "game" ]]; then
    BUILD="$ROOT/build/wii-release"
    BRINGUP=OFF
else
    BUILD="$ROOT/build/wii-bringup"
    BRINGUP=ON
fi

(( CLEAN )) && rm -rf "$BUILD"

ARGS=(-DCMAKE_BUILD_TYPE=Release -DWII_BRINGUP="$BRINGUP" -DWII_ENABLE_SOUND=ON -DWII_ENABLE_NETWORK=ON "${EXTRA[@]}")

configure_wii "$BUILD" "${ARGS[@]}"

if (( DATA )); then
    cmake --build "$BUILD" --target wii-data --parallel "$(jobs)"
    echo "Wii data staged under: $ROOT/bin/wii/sd"
    exit 0
fi

cmake --build "$BUILD" --target OptiCraft --parallel "$(jobs)"
echo
echo "Wii build completed."
echo "ELF: $ROOT/bin/wii/OptiCraft.elf"
echo "SD:  $ROOT/bin/wii/sd"
