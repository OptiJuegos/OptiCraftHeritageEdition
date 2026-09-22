#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd)"

AUTO_YES="${OPTICRAFT_AUTO_YES:-0}"

fail() {
    echo "ERROR: $*" >&2
    exit 1
}

ask_yes_no() {
    local prompt="$1"
    if [[ "$AUTO_YES" == "1" ]]; then
        echo "$prompt [auto: yes]"
        return 0
    fi
    [[ -t 0 ]] || return 1
    local answer
    read -r -p "$prompt [Y/n] " answer
    case "${answer:-Y}" in
        Y|y|YES|Yes|yes|S|s|SIM|Sim|sim) return 0 ;;
        *) return 1 ;;
    esac
}

as_root() {
    if [[ ${EUID:-$(id -u)} -eq 0 ]]; then
        "$@"
    elif command -v sudo >/dev/null 2>&1; then
        sudo "$@"
    else
        fail "This operation needs root privileges and sudo is not installed."
    fi
}

command_exists() { command -v "$1" >/dev/null 2>&1; }

jobs() {
    if [[ -n "${JOBS:-}" ]]; then
        printf '%s\n' "$JOBS"
    elif command_exists nproc; then
        nproc
    else
        echo 4
    fi
}

install_base_tools() {
    echo "Installing common build dependencies..."
    if command_exists pacman; then
        as_root pacman -S --needed --noconfirm cmake ninja python git curl tar zip gcc
    elif command_exists apt-get; then
        as_root apt-get update
        as_root apt-get install -y cmake ninja-build python3 git curl tar zip build-essential
    elif command_exists dnf; then
        as_root dnf install -y cmake ninja-build python3 git curl tar zip gcc gcc-c++
    elif command_exists zypper; then
        as_root zypper --non-interactive install cmake ninja python3 git curl tar zip gcc gcc-c++
    elif command_exists apk; then
        as_root apk add cmake ninja python3 git curl tar zip build-base
    elif command_exists xbps-install; then
        as_root xbps-install -Sy cmake ninja python3 git curl tar zip gcc
    else
        fail "Unsupported package manager. Install CMake, Ninja, Python 3 and a C/C++ compiler manually."
    fi
}

ensure_base_tools() {
    local missing=()
    command_exists cmake || missing+=("cmake")
    command_exists ninja || missing+=("ninja")
    command_exists python3 || missing+=("python3")
    command_exists gcc || missing+=("gcc")
    command_exists g++ || missing+=("g++")

    if ((${#missing[@]})); then
        echo "Missing tools: ${missing[*]}"
        if ask_yes_no "Install the missing Linux build tools now?"; then
            install_base_tools
        else
            fail "Required build tools are missing."
        fi
    fi
}

ensure_zip_tool() {
    if command_exists zip; then return 0; fi
    if ask_yes_no "The zip command is missing. Install it now?"; then
        if command_exists pacman; then as_root pacman -S --needed --noconfirm zip
        elif command_exists apt-get; then as_root apt-get update; as_root apt-get install -y zip
        elif command_exists dnf; then as_root dnf install -y zip
        elif command_exists zypper; then as_root zypper --non-interactive install zip
        elif command_exists apk; then as_root apk add zip
        elif command_exists xbps-install; then as_root xbps-install -Sy zip
        else fail "Install zip manually."
        fi
    else
        fail "zip is required."
    fi
}

ps2dev_valid() {
    local p="$1"
    [[ -n "$p" ]] || return 1
    [[ -x "$p/ee/bin/mips64r5900el-ps2-elf-g++" ]] || return 1
    [[ -d "$p/ps2sdk" ]] || return 1
    [[ -d "$p/gsKit" ]] || return 1
}

ensure_ps2dev() {
    ensure_base_tools

    local requested="${PS2DEV:-}"
    if ps2dev_valid "$requested"; then
        :
    elif ps2dev_valid "/usr/local/ps2dev"; then
        PS2DEV="/usr/local/ps2dev"
    else
        PS2DEV="${requested:-/usr/local/ps2dev}"
        echo "PS2DEV was not found or is incomplete at: $PS2DEV"
        if ! ask_yes_no "Download and install the official precompiled PS2DEV toolchain?"; then
            fail "PS2DEV is required for PS2 builds."
        fi

        command_exists curl || install_base_tools
        command_exists tar || install_base_tools

        if [[ -e "$PS2DEV" && -n "$(find "$PS2DEV" -mindepth 1 -maxdepth 1 -print -quit 2>/dev/null || true)" ]]; then
            if ask_yes_no "Remove the incomplete PS2DEV directory before reinstalling?"; then
                as_root rm -rf -- "$PS2DEV"
            else
                fail "Cannot install over an incomplete PS2DEV directory."
            fi
        fi

        as_root mkdir -p "$PS2DEV"
        if [[ ! -w "$PS2DEV" ]]; then
            as_root chown -R "$(id -u):$(id -g)" "$PS2DEV"
        fi

        local archive
        archive="$(mktemp --suffix=.tar.gz ps2dev-XXXXXX)"
        curl -fL --retry 3 --retry-delay 2 \
            -o "$archive" \
            "https://github.com/ps2dev/ps2dev/releases/download/latest/ps2dev-ubuntu-latest.tar.gz"
        tar -xzf "$archive" --strip-components=1 -C "$PS2DEV"
        rm -f "$archive"
    fi

    ps2dev_valid "$PS2DEV" || fail "PS2DEV installation is incomplete: $PS2DEV"

    export PS2DEV
    export PS2SDK="$PS2DEV/ps2sdk"
    export GSKIT="$PS2DEV/gsKit"
    export PATH="$PATH:$PS2DEV/bin:$PS2DEV/ee/bin:$PS2DEV/iop/bin:$PS2DEV/dvp/bin:$PS2SDK/bin"
}

devkitpro_valid() {
    local root="${DEVKITPRO:-/opt/devkitpro}"
    [[ -x "$root/devkitPPC/bin/powerpc-eabi-g++" ]] || return 1
    [[ -d "$root/libogc" || -d "$root/wut" || -d "$root/portlibs" ]] || return 1
}

ensure_wii_toolchain() {
    ensure_base_tools
    DEVKITPRO="${DEVKITPRO:-/opt/devkitpro}"

    if devkitpro_valid; then
        export DEVKITPRO
        export DEVKITPPC="${DEVKITPPC:-$DEVKITPRO/devkitPPC}"
        export PATH="$DEVKITPPC/bin:$DEVKITPRO/tools/bin:$PATH"
        return 0
    fi

    echo "devkitPPC/Wii development packages were not found under $DEVKITPRO."
    if ! ask_yes_no "Install the official devkitPro Wii toolchain now?"; then
        fail "devkitPPC + wii-dev are required for Wii builds."
    fi

    if command_exists dkp-pacman; then
        as_root dkp-pacman -S --needed --noconfirm wii-dev
    elif command_exists apt-get; then
        command_exists curl || install_base_tools
        local installer
        installer="$(mktemp)"
        curl -fL -o "$installer" "https://apt.devkitpro.org/install-devkitpro-pacman"
        chmod +x "$installer"
        as_root "$installer"
        rm -f "$installer"
        as_root dkp-pacman -S --needed --noconfirm wii-dev
    else
        echo
        echo "Automatic devkitPro bootstrap is only enabled here when dkp-pacman"
        echo "is already available, or on Debian/Ubuntu systems."
        echo "Follow the official devkitPro pacman setup for your distro, then run:"
        echo "  sudo dkp-pacman -S wii-dev"
        echo "  https://devkitpro.org/wiki/devkitPro_pacman"
        exit 1
    fi

    devkitpro_valid || fail "Wii toolchain installation did not complete successfully."
    export DEVKITPRO
    export DEVKITPPC="${DEVKITPPC:-$DEVKITPRO/devkitPPC}"
    export PATH="$DEVKITPPC/bin:$DEVKITPRO/tools/bin:$PATH"
}

can_build_32bit() {
    local tmp
    tmp="$(mktemp --suffix=.c)"
    printf 'int main(void){return 0;}\n' > "$tmp"
    if gcc -m32 "$tmp" -o "${tmp%.c}" >/dev/null 2>&1; then
        rm -f "$tmp" "${tmp%.c}"
        return 0
    fi
    rm -f "$tmp" "${tmp%.c}"
    return 1
}

install_multilib() {
    if command_exists pacman; then
        as_root pacman -S --needed --noconfirm lib32-glibc lib32-gcc-libs
    elif command_exists apt-get; then
        as_root apt-get update
        as_root apt-get install -y gcc-multilib g++-multilib
    elif command_exists dnf; then
        as_root dnf install -y glibc-devel.i686 libstdc++-devel.i686
    elif command_exists zypper; then
        as_root zypper --non-interactive install glibc-devel-32bit libstdc++6-devel-gcc-32bit
    else
        fail "Automatic multilib installation is not supported for this distro."
    fi
}

ensure_32bit_toolchain() {
    ensure_base_tools
    if can_build_32bit; then return 0; fi
    if ask_yes_no "32-bit GCC support is missing. Install multilib packages now?"; then
        install_multilib
    else
        fail "32-bit GCC support is required."
    fi
    can_build_32bit || fail "GCC still cannot build 32-bit executables after installation."
}

configure_native() {
    local build_dir="$1"
    shift
    ensure_base_tools
    cmake -S "$ROOT" -B "$build_dir" -G Ninja \
        -DSDL_PIPEWIRE=OFF \
        "$@"
}

build_native() {
    local build_dir="$1"
    shift
    configure_native "$build_dir" "$@"
    cmake --build "$build_dir" --target OptiCraft --parallel "$(jobs)"
}

configure_ps2() {
    local build_dir="$1"
    shift
    ensure_ps2dev
    cmake -S "$ROOT" -B "$build_dir" -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="$ROOT/cmake/ps2_toolchain.cmake" \
        -DPLATFORM=PS2 \
        "$@"
}

build_ps2_profile() {
    local build_dir="$1"
    shift
    configure_ps2 "$build_dir" "$@"
    cmake --build "$build_dir" --target OptiCraft --parallel "$(jobs)"
}

stage_ps2_data() {
    local build_dir="$1"
    shift
    configure_ps2 "$build_dir" "$@"
    cmake --build "$build_dir" --target ps2-data --parallel "$(jobs)"
    if [[ -f "$ROOT/scripts/make_pak.py" && -d "$ROOT/bin/ps2/usb/MCBETA/data" ]]; then
        python3 "$ROOT/scripts/make_pak.py" \
            "$ROOT/bin/ps2/usb/MCBETA/data" \
            "$ROOT/bin/ps2/usb/MCBETA/assets.pak"
    fi
}

configure_wii() {
    local build_dir="$1"
    shift
    ensure_wii_toolchain
    cmake -S "$ROOT" -B "$build_dir" -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="$ROOT/cmake/wii_toolchain.cmake" \
        -DPLATFORM=WII \
        -DDEVKITPRO="$DEVKITPRO" \
        "$@"
}
