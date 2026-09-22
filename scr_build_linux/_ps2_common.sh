#!/usr/bin/env bash
set -Eeuo pipefail

# Shared helpers for Linux ports of the original OptiJuegos PS2 build scripts.
# Expected location: <project>/scr_build_linux/

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd)"

export PS2DEV="${PS2DEV:-/usr/local/ps2dev}"
export PS2SDK="${PS2SDK:-$PS2DEV/ps2sdk}"
export GSKIT="${GSKIT:-$PS2DEV/gsKit}"
export PATH="$PATH:$PS2DEV/bin:$PS2DEV/ee/bin:$PS2DEV/iop/bin:$PS2DEV/dvp/bin:$PS2SDK/bin"

JOBS="${JOBS:-}"
if [[ -z "$JOBS" ]]; then
    if command -v nproc >/dev/null 2>&1; then
        JOBS="$(nproc)"
    else
        JOBS=4
    fi
fi

ps2_fail() {
    echo "ERROR: $*" >&2
    exit 1
}

ps2_require_environment() {
    command -v cmake >/dev/null 2>&1 || \
        ps2_fail "CMake not found. Run ./build_ps2_linux.sh first."
    command -v ninja >/dev/null 2>&1 || \
        ps2_fail "Ninja not found. Run ./build_ps2_linux.sh first."
    command -v python3 >/dev/null 2>&1 || \
        ps2_fail "Python 3 not found. Run ./build_ps2_linux.sh first."

    [[ -f "$ROOT/CMakeLists.txt" ]] || ps2_fail "Project root not found: $ROOT"
    [[ -f "$ROOT/cmake/ps2_toolchain.cmake" ]] || ps2_fail "PS2 CMake toolchain not found."
    [[ -x "$PS2DEV/ee/bin/mips64r5900el-ps2-elf-g++" ]] || \
        ps2_fail "PS2 EE compiler not found under $PS2DEV"
    [[ -d "$PS2SDK" ]] || ps2_fail "PS2SDK not found: $PS2SDK"
}

ps2_maybe_clean() {
    local build_dir="$1"
    local arg="${2:-}"
    if [[ "${arg,,}" == "clean" ]]; then
        echo "Removing $build_dir ..."
        rm -rf "$build_dir"
    fi
}

ps2_configure() {
    local build_dir="$1"
    shift
    cmake -S "$ROOT" -B "$build_dir" -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="$ROOT/cmake/ps2_toolchain.cmake" \
        -DPLATFORM=PS2 \
        "$@"
}

ps2_stage_data() {
    local build_dir="$1"
    shift
    ps2_require_environment
    ps2_configure "$build_dir" "$@"
    cmake --build "$build_dir" --target ps2-data --parallel "$JOBS"
    echo
    echo "PS2 staged data:"
    echo "  $ROOT/bin/ps2/usb/MCBETA/data"
}

ps2_make_pak_if_possible() {
    local app_dir="$ROOT/bin/ps2/usb/MCBETA"
    local pak="$app_dir/assets.pak"

    [[ -d "$app_dir/data" ]] || return 0
    [[ -f "$ROOT/scripts/make_pak.py" ]] || return 0

    python3 "$ROOT/scripts/make_pak.py" "$app_dir/data" "$pak"
    echo "PAK: $pak"
}

ps2_build_profile() {
    local build_dir="$1"
    shift
    ps2_require_environment

    echo "Project: $ROOT"
    echo "PS2DEV:  $PS2DEV"
    echo "Build:   $build_dir"
    echo "Jobs:    $JOBS"
    echo

    ps2_configure "$build_dir" "$@"
    cmake --build "$build_dir" --target OptiCraft --parallel "$JOBS"

    echo
    echo "Build completed."
    if [[ -f "$ROOT/bin/ps2/usb/MCBETA/OptiCraft.elf" ]]; then
        echo "ELF: $ROOT/bin/ps2/usb/MCBETA/OptiCraft.elf"
    elif [[ -f "$ROOT/bin/ps2/OptiCraft.elf" ]]; then
        echo "ELF: $ROOT/bin/ps2/OptiCraft.elf"
    fi
}
