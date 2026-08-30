# wii_toolchain.cmake — devkitPPC / libogc cross toolchain for the Nintendo Wii.
#
# Self-contained on purpose. devkitPro ships its own $DEVKITPRO/cmake/Wii.cmake,
# but its contents have changed across releases (and it bakes SDK paths into the
# CMAKE_*_FLAGS *strings*, the same trap that forced cmake/ps2_toolchain.cmake to
# exist for ps2dev). Defining the toolchain here keeps the Wii build reproducible
# against any devkitPPC install and keeps every SDK path out of the flag strings,
# so a source tree under "MC BETA 1.7.3 SOURCE CODE JAVA" cannot shatter them.
#
# Resolution order for the SDK root:
#   1. -DDEVKITPRO=<path> on the CMake command line
#   2. $ENV{DEVKITPRO}
#   3. the usual per-host default (C:/devkitPro on Windows, /opt/devkitpro else)

# CMake re-includes the toolchain file for every try_compile; keep it idempotent.
if(WII_TOOLCHAIN_INCLUDED)
    return()
endif()
set(WII_TOOLCHAIN_INCLUDED YES)

set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_VERSION   1)
set(CMAKE_SYSTEM_PROCESSOR powerpc)

# --- Host/generator sanity ----------------------------------------------------
# The wii-* presets point CMAKE_MAKE_PROGRAM at the ninja.exe bundled in the
# repository, which is a native Windows binary. Driving it from devkitPro's MSYS2
# cmake instead of the Windows one produces a build.ninja full of POSIX paths
# (/d/MC/...) that the Windows ninja reads as \d\MC\..., and the build dies with
#
#     ninja: error: 'testCCompiler.c' ... missing and no known rule to make it
#     -- Check for working C compiler: ... - broken
#
# which reads as a broken cross compiler when nothing is wrong with it.
#
# Detection uses CMAKE_COMMAND rather than CMAKE_HOST_WIN32: this file is
# included from the generated CMakeSystem.cmake, at which point the CMAKE_HOST_*
# variables are not populated yet, so testing them silently matches everything.
# A cmake whose own executable path is POSIX-rooted is an MSYS2 cmake.
#
# WARNING, not FATAL_ERROR, and deliberately so -- a heuristic that blocks a
# perfectly good build is worse than the cryptic error it was meant to explain.
# If the guess is right the build fails anyway, now with this printed above it.
if(DEFINED CMAKE_MAKE_PROGRAM
   AND CMAKE_MAKE_PROGRAM MATCHES "McBetaCpp/ninja\\.exe$"
   AND CMAKE_COMMAND MATCHES "^/")
    message(WARNING
        "Possible path-convention mismatch: a POSIX-style CMake (${CMAKE_COMMAND}) "
        "is driving the repository's native Windows ninja.exe.\n"
        "Two different failures come out of this, and neither names the real "
        "cause -- in both the compiler is fine and the paths are not:\n"
        "  * \"The CMAKE_C_COMPILER: C is not a full path\" -- a POSIX CMake does "
        "not read C:/... as absolute, so the drive letter is all that survives.\n"
        "  * \"testCCompiler.c missing and no known rule to make it\", reported as "
        "\"the C compiler is broken\" -- POSIX paths written into build.ninja.\n"
        "Fix:\n"
        "  1. Build from cmd/PowerShell:  build wii.bat game\n"
        "     (extra options pass through: build wii.bat game -DFOO=ON)\n"
        "  2. Or stay in MSYS2 with its own ninja:\n"
        "       pacman -S ninja\n"
        "       cmake --preset wii-release -DCMAKE_MAKE_PROGRAM=ninja\n"
        "Either way, WIPE THE BUILD DIRECTORY FIRST. A run that got this far has "
        "already rewritten CMakeCache.txt with this CMake's conventions "
        "(CMAKE_COMMAND, CMAKE_MAKE_PROGRAM, CMAKE_TOOLCHAIN_FILE), and the next "
        "configure inherits them whichever cmake you use: build wii.bat clean game")
endif()

# Marks the build for CMakeLists.txt, which dispatches to cmake/wii.cmake.
set(NINTENDO_WII YES)

# --- SDK root -----------------------------------------------------------------
# Resolution is by "first candidate that is actually a directory" rather than by
# host detection, because the CMAKE_HOST_* variables are not populated yet when
# this file is included (see the note above) -- keying off them silently picks
# the wrong branch.
#
# The case this exists for: devkitPro's installer exports DEVKITPRO=/opt/devkitpro
# even into the *Windows* environment, where it is an MSYS2-only path that native
# CMake cannot open. So an explicitly-provided value is preferred but not
# trusted; if it does not resolve, the real install locations are tried.
set(_dkp_candidates "")
if(DEFINED DEVKITPRO)
    list(APPEND _dkp_candidates "${DEVKITPRO}")
endif()
if(DEFINED ENV{DEVKITPRO})
    list(APPEND _dkp_candidates "$ENV{DEVKITPRO}")
endif()
list(APPEND _dkp_candidates "C:/devkitPro" "/opt/devkitpro")

set(_dkp_found "")
foreach(_cand IN LISTS _dkp_candidates)
    string(REPLACE "\\" "/" _cand "${_cand}")
    # devkitPPC is the marker: a bare directory that happens to exist is not an
    # install, and picking one would fail later with a far worse message.
    if(IS_DIRECTORY "${_cand}/devkitPPC")
        set(_dkp_found "${_cand}")
        break()
    endif()
endforeach()

if(NOT _dkp_found)
    message(FATAL_ERROR
        "devkitPro not found. Tried: ${_dkp_candidates}\n"
        "Install it (see src/wii/README_wii.md) or pass -DDEVKITPRO=<path>.")
endif()
set(DEVKITPRO "${_dkp_found}")

set(DEVKITPPC "${DEVKITPRO}/devkitPPC" CACHE PATH "devkitPPC root")
set(LIBOGC    "${DEVKITPRO}/libogc"    CACHE PATH "libogc root")

# devkitPro splits PowerPC portlibs across two prefixes and which one a package
# lands in depends on the package, not on the target: `ppc-*` packages (zlib,
# libpng, ...) install under portlibs/ppc, while `wii-*` packages install under
# portlibs/wii. Search both rather than guessing.
set(PPC_PORTLIBS "${DEVKITPRO}/portlibs/ppc" CACHE PATH "Shared PowerPC portlibs root")
set(WII_PORTLIBS "${DEVKITPRO}/portlibs/wii" CACHE PATH "Wii-specific portlibs root")

if(NOT IS_DIRECTORY "${DEVKITPPC}")
    message(FATAL_ERROR "devkitPPC not found at '${DEVKITPPC}'. Install the 'wii-dev' package group.")
endif()
if(NOT IS_DIRECTORY "${LIBOGC}")
    message(FATAL_ERROR "libogc not found at '${LIBOGC}'. Install the 'wii-dev' package group.")
endif()

message(STATUS "Wii build: DEVKITPRO=${DEVKITPRO}")
message(STATUS "Wii build: DEVKITPPC=${DEVKITPPC}")
message(STATUS "Wii build: LIBOGC=${LIBOGC}")

# --- Compilers ----------------------------------------------------------------
# Probe for the suffix instead of deriving it from the host, for the third time
# in this file the same reason: CMAKE_HOST_WIN32 is not populated this early, so
# a host test silently yields the empty suffix and the EXISTS check below then
# rejects a perfectly good toolchain.
if(EXISTS "${DEVKITPPC}/bin/powerpc-eabi-gcc.exe")
    set(_dkp_exe ".exe")
else()
    set(_dkp_exe "")
endif()

set(CMAKE_C_COMPILER   "${DEVKITPPC}/bin/powerpc-eabi-gcc${_dkp_exe}"     CACHE FILEPATH "")
set(CMAKE_CXX_COMPILER "${DEVKITPPC}/bin/powerpc-eabi-g++${_dkp_exe}"     CACHE FILEPATH "")
set(CMAKE_ASM_COMPILER "${DEVKITPPC}/bin/powerpc-eabi-gcc${_dkp_exe}"     CACHE FILEPATH "")
set(CMAKE_AR           "${DEVKITPPC}/bin/powerpc-eabi-ar${_dkp_exe}"      CACHE FILEPATH "")
set(CMAKE_RANLIB       "${DEVKITPPC}/bin/powerpc-eabi-ranlib${_dkp_exe}"  CACHE FILEPATH "")
set(CMAKE_OBJCOPY      "${DEVKITPPC}/bin/powerpc-eabi-objcopy${_dkp_exe}" CACHE FILEPATH "")
set(CMAKE_STRIP        "${DEVKITPPC}/bin/powerpc-eabi-strip${_dkp_exe}"   CACHE FILEPATH "")

if(NOT EXISTS "${CMAKE_CXX_COMPILER}")
    message(FATAL_ERROR "powerpc-eabi-g++ not found at '${CMAKE_CXX_COMPILER}'.")
endif()

# --- Machine flags ------------------------------------------------------------
# Mirrors $DEVKITPRO/devkitPPC/wii_rules (MACHDEP). Every one of these is
# load-bearing:
#   -DHW_RVL      libogc's own switch between GameCube (HW_DOL) and Wii builds;
#                 it selects the Wii memory map, MEM2, WPAD and the IOS paths.
#   -mrvl         devkitPPC target flag: picks the Wii crt0, specs and linker
#                 script (load address 0x80004000, MEM1/MEM2 layout).
#   -mcpu=750     Broadway is a PowerPC 750CL. Paired-singles are NOT enabled by
#                 this flag; -mpaired is a separate opt-in and is left off until
#                 there is a measured hot loop to justify it.
#   -meabi        the embedded ABI libogc and its libraries are compiled against.
#   -mhard-float  the 750CL has a real FPU with hardware doubles. This is the
#                 single biggest difference from the EE, where `double` is
#                 software-emulated (see PS2_FLOAT_ORE_VEINS).
set(_WII_MACHDEP "-DHW_RVL -mrvl -mcpu=750 -meabi -mhard-float")

set(CMAKE_C_FLAGS_INIT   "${_WII_MACHDEP}")
set(CMAKE_CXX_FLAGS_INIT "${_WII_MACHDEP}")
set(CMAKE_ASM_FLAGS_INIT "${_WII_MACHDEP}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${_WII_MACHDEP}")

# --- Cross-compile lookup rules -----------------------------------------------
set(CMAKE_FIND_ROOT_PATH "${DEVKITPPC}" "${LIBOGC}" "${WII_PORTLIBS}" "${PPC_PORTLIBS}")
# PROGRAM=NEVER so host tools (elf2dol, ninja) are still found on the host PATH.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# A bare `int main(){}` cannot be linked without libogc's crt0 + libs, which are
# not on the link line during the compiler check. Probe with a static library
# instead, exactly as the PS2 preset does.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
