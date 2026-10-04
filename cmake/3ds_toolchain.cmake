# 3ds_toolchain.cmake — devkitARM / libctru cross toolchain for the Nintendo 3DS.
#
# Self-contained on purpose, for the same reasons cmake/wii_toolchain.cmake is:
# devkitPro ships its own $DEVKITPRO/cmake/3DS.cmake, but its contents have
# changed across releases and the devkitPro toolchain files bake SDK paths into
# the CMAKE_*_FLAGS *strings*. Defining the toolchain here keeps the 3DS build
# reproducible against any devkitARM install and keeps every SDK path out of the
# flag strings, so a source tree with spaces in its name cannot shatter them.
#
# The machine flags mirror devkitPro's own template (3ds-examples,
# templates/application/Makefile), which is the configuration libctru and the
# prebuilt citro3d/citro2d archives are compiled against:
#
#   ARCH  = -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
#           armv6k = the ARM11 MPCore in the 3DS; -mtp=soft because the kernel
#           owns TPIDRURW and newlib takes the thread pointer from __service_loop.
#   CFLAGS add -D__3DS__ (the single macro libctru-era code keys off) and
#           -mword-relocations (the 3DS loader relocates .text; PIE-less but
#           PIC-ish code can reference read-only data through relative offsets).
#   link:  -specs=3dsx.specs -- added by cmake/3ds.cmake on the target, not
#           here, because it only matters for the final executable link.
#
# Exceptions and RTTI stay ON (GCC's defaults). devkitPro's template disables
# them (-fno-rtti -fno-exceptions); this codebase uses both, the same way the
# Wii build passes -frtti explicitly.
#
# Resolution order for the SDK root:
#   1. -DDEVKITPRO=<path> on the CMake command line
#   2. $ENV{DEVKITPRO}
#   3. the usual per-host default (C:/devkitPro on Windows, /opt/devkitpro else)

# CMake re-includes the toolchain file for every try_compile; keep it idempotent.
if(DS_TOOLCHAIN_INCLUDED)
    return()
endif()
set(DS_TOOLCHAIN_INCLUDED YES)

set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_VERSION   1)
set(CMAKE_SYSTEM_PROCESSOR arm)

# --- Host/generator sanity ----------------------------------------------------
# The 3ds-* presets point CMAKE_MAKE_PROGRAM at the ninja.exe bundled in the
# repository, which is a native Windows binary. Driving it from devkitPro's MSYS2
# cmake instead of the Windows one produces a build.ninja full of POSIX paths
# that the Windows ninja misreads, and the build dies with a false
# "the C compiler is broken". Same trap cmake/wii_toolchain.cmake documents;
# detection uses CMAKE_COMMAND because CMAKE_HOST_* is not populated this early.
#
# WARNING, not FATAL_ERROR: a heuristic that blocks a perfectly good build is
# worse than the cryptic error it was meant to explain.
if(DEFINED CMAKE_MAKE_PROGRAM
   AND CMAKE_MAKE_PROGRAM MATCHES "ninja\\.exe$"
   AND CMAKE_COMMAND MATCHES "^/")
    message(WARNING
        "Possible path-convention mismatch: a POSIX-style CMake (${CMAKE_COMMAND}) "
        "is driving the repository's native Windows ninja.exe.\n"
        "Fix: configure from cmd/PowerShell (native Windows cmake), not from the "
        "devkitPro MSYS2 shell -- or pass -DCMAKE_MAKE_PROGRAM=ninja from MSYS2. "
        "Either way, WIPE build/3ds-* first: a run that got this far already "
        "rewrote CMakeCache.txt with this CMake's conventions.")
endif()

# Marks the build for CMakeLists.txt, which dispatches to cmake/3ds.cmake.
set(NINTENDO_3DS YES)

# --- SDK root -----------------------------------------------------------------
# Resolution is by "first candidate that is actually a directory" rather than by
# host detection (the CMAKE_HOST_* note above applies here too). devkitPro's
# installer exports DEVKITPRO=/opt/devkitpro even into the *Windows* environment,
# where it is an MSYS2-only path native CMake cannot open -- so an
# explicitly-provided value is preferred but not trusted; if it does not
# resolve, the real install locations are tried. The marker is devkitARM itself:
# a bare directory that happens to exist is not an install.
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
    if(IS_DIRECTORY "${_cand}/devkitARM")
        set(_dkp_found "${_cand}")
        break()
    endif()
endforeach()

if(NOT _dkp_found)
    message(FATAL_ERROR
        "devkitPro/devkitARM not found. Tried: ${_dkp_candidates}\n"
        "Install it with the devkitPro Windows installer (select the "
        "'3DS Development' component), or from a unix shell with "
        "'dkp-pacman -S 3ds-dev'. You can also pass -DDEVKITPRO=<path>.")
endif()
set(DEVKITPRO "${_dkp_found}")

set(DEVKITARM     "${DEVKITPRO}/devkitARM"      CACHE PATH "devkitARM root")
set(LIBCTRU       "${DEVKITPRO}/libctru"        CACHE PATH "libctru root (also ships citro3d/citro2d headers)")
set(THREEDS_PORTLIBS "${DEVKITPRO}/portlibs/3ds" CACHE PATH "3DS-specific portlibs root")

if(NOT IS_DIRECTORY "${DEVKITARM}")
    message(FATAL_ERROR "devkitARM not found at '${DEVKITARM}'. Install the '3DS Development' component / '3ds-dev' package group.")
endif()
if(NOT IS_DIRECTORY "${LIBCTRU}")
    message(FATAL_ERROR "libctru not found at '${LIBCTRU}'. Install the '3DS Development' component / '3ds-dev' package group.")
endif()

message(STATUS "3DS build: DEVKITPRO=${DEVKITPRO}")
message(STATUS "3DS build: DEVKITARM=${DEVKITARM}")
message(STATUS "3DS build: LIBCTRU=${LIBCTRU}")

# --- Compilers ----------------------------------------------------------------
# Probe for the suffix instead of deriving it from the host: CMAKE_HOST_WIN32 is
# not populated this early, and testing it silently yields the empty suffix and
# then rejects a perfectly good toolchain.
if(EXISTS "${DEVKITARM}/bin/arm-none-eabi-gcc.exe")
    set(_dkp_exe ".exe")
else()
    set(_dkp_exe "")
endif()

set(CMAKE_C_COMPILER   "${DEVKITARM}/bin/arm-none-eabi-gcc${_dkp_exe}"     CACHE FILEPATH "")
set(CMAKE_CXX_COMPILER "${DEVKITARM}/bin/arm-none-eabi-g++${_dkp_exe}"     CACHE FILEPATH "")
set(CMAKE_ASM_COMPILER "${DEVKITARM}/bin/arm-none-eabi-gcc${_dkp_exe}"     CACHE FILEPATH "")
set(CMAKE_AR           "${DEVKITARM}/bin/arm-none-eabi-ar${_dkp_exe}"      CACHE FILEPATH "")
set(CMAKE_RANLIB       "${DEVKITARM}/bin/arm-none-eabi-ranlib${_dkp_exe}"  CACHE FILEPATH "")
set(CMAKE_OBJCOPY      "${DEVKITARM}/bin/arm-none-eabi-objcopy${_dkp_exe}" CACHE FILEPATH "")
set(CMAKE_STRIP        "${DEVKITARM}/bin/arm-none-eabi-strip${_dkp_exe}"   CACHE FILEPATH "")

if(NOT EXISTS "${CMAKE_CXX_COMPILER}")
    message(FATAL_ERROR "arm-none-eabi-g++ not found at '${CMAKE_CXX_COMPILER}'.")
endif()

# --- Machine flags ------------------------------------------------------------
# Mirrors the template's ARCH + CFLAGS (see the header comment). Every flag here
# is load-bearing; -mfloat-abi=hard in particular must match the prebuilt
# newlib/libctru/citro3d archives, which are all hard-float.
set(_3DS_ARCH "-D__3DS__ -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft -mword-relocations")

set(CMAKE_C_FLAGS_INIT           "${_3DS_ARCH}")
set(CMAKE_CXX_FLAGS_INIT         "${_3DS_ARCH}")
set(CMAKE_ASM_FLAGS_INIT         "${_3DS_ARCH}")
# Link line gets the arch (it selects the multilib/crt0 variant) but not the
# -D/-mword-relocations; -specs=3dsx.specs is added by cmake/3ds.cmake.
set(CMAKE_EXE_LINKER_FLAGS_INIT  "-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft")

# --- Cross-compile lookup rules -----------------------------------------------
set(CMAKE_FIND_ROOT_PATH "${DEVKITARM}" "${LIBCTRU}" "${THREEDS_PORTLIBS}")
# PROGRAM=NEVER so host tools (3dsxtool, smdhtool, ninja) are still found.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# A bare `int main(){}` cannot be linked without libctru's crt0 + specs, which
# are not on the link line during the compiler check. Probe with a static
# library instead, exactly as the PS2/Wii presets do.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
