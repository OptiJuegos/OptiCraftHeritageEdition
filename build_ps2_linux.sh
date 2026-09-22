#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
SCRIPTS_DIR="$ROOT/scr_build_linux"
AUTO_YES=0
CLEAN=0

usage() {
    cat <<'EOF'
Usage: ./build_ps2_linux.sh [--clean] [--yes]

Without a selection argument, an interactive menu is shown.

  --clean   Clean build/ps2 when using the standard PS2 build (option 0).
  --yes     Automatically accept supported dependency installation prompts.
  -h        Show this help.
EOF
}

for arg in "$@"; do
    case "$arg" in
        --clean|clean) CLEAN=1 ;;
        --yes|-y) AUTO_YES=1 ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown option: $arg" >&2; usage; exit 2 ;;
    esac
done

[[ -f "$ROOT/CMakeLists.txt" ]] || {
    echo "ERROR: place build_ps2_linux.sh in the project root." >&2
    exit 1
}
[[ -d "$SCRIPTS_DIR" ]] || {
    echo "ERROR: scr_build_linux was not found next to this script." >&2
    exit 1
}

export OPTICRAFT_AUTO_YES="$AUTO_YES"
source "$SCRIPTS_DIR/_build_common.sh"

mapfile -d '' BUILD_SCRIPTS < <(
    find "$SCRIPTS_DIR" -maxdepth 1 -type f -name '*.sh' \
        ! -name '_build_common.sh' \
        ! -name '!!del GUARDA!!.sh' \
        -print0 | sort -z
)

echo
echo "============================================================"
echo "OptiCraft / MC:PS2 - Linux build selection"
echo "============================================================"
echo "[0] Standard automatic PS2 build (network + sound)"
for i in "${!BUILD_SCRIPTS[@]}"; do
    printf '[%d] %s\n' "$((i + 1))" "$(basename "${BUILD_SCRIPTS[$i]}")"
done
echo "[Q] Quit"
echo

while true; do
    read -r -p "Select a script [0]: " choice
    choice="${choice:-0}"
    case "${choice,,}" in
        q) exit 0 ;;
        0)
            BUILD="$ROOT/build/ps2"
            (( CLEAN )) && rm -rf "$BUILD"
            ensure_ps2dev
            ARGS=(
                -DCMAKE_BUILD_TYPE=Release
                -DPS2_ENABLE_NETWORK=ON
                -DPS2_ENABLE_SOUND=ON
                -DMC_LOG_LEVEL=0
                -DPS2_RENDER_STATS=OFF
                -DPS2_ENABLE_PERSPECTIVE_TEXTURES=ON
                -DPS2_ENABLE_VU1_TERRAIN=ON
            )
            build_ps2_profile "$BUILD" "${ARGS[@]}"
            stage_ps2_data "$BUILD" "${ARGS[@]}"
            echo
            echo "Standard PS2 build completed."
            echo "ELF: $ROOT/bin/ps2/usb/MCBETA/OptiCraft.elf"
            echo "PAK: $ROOT/bin/ps2/usb/MCBETA/assets.pak"
            exit 0
            ;;
    esac

    if [[ "$choice" =~ ^[0-9]+$ ]] &&
       (( choice >= 1 && choice <= ${#BUILD_SCRIPTS[@]} )); then
        selected="${BUILD_SCRIPTS[$((choice - 1))]}"
        echo
        echo "Running: $(basename "$selected")"
        echo
        cd "$ROOT"
        bash "$selected"
        exit $?
    fi

    echo "Invalid selection."
done
