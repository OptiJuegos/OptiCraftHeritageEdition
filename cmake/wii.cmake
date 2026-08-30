# wii.cmake — Nintendo Wii (Broadway / Hollywood) build branch for BetaPlusPlus.
#
# Included from the top of CMakeLists.txt when cmake/wii_toolchain.cmake is
# active (it sets NINTENDO_WII), which then return()s so none of the desktop
# SDL2/glad/OpenGL configuration runs. Structured to mirror cmake/ps2.cmake.
#
# Two build shapes are available through presets:
#
#   wii-release / wii-debug
#       Build the full game. This is the normal shipping/development path.
#
#   wii-bringup
#       Builds only the minimal toolchain/hardware smoke test. It proves that
#       devkitPPC/libogc/libfat can link and boot without pulling game code.
#
# Output: bin/wii/BetaPlusPlus.elf plus an SD-card-shaped tree under bin/wii/sd/
# (apps/BetaPlusPlus/boot.dol + meta.xml). Copy bin/wii/sd/* to the root of an
# SD card, or point Dolphin straight at the .elf.

cmake_minimum_required(VERSION 3.21)

include(${CMAKE_SOURCE_DIR}/cmake/SourceSelection.cmake)

# --- Wii feature options ------------------------------------------------------
option(WII_BRINGUP "Build only the Wii toolchain smoke test instead of the game" OFF)
option(WII_ENABLE_SOUND "Enable the libogc ASND sound backend" ON)
option(WII_ENABLE_NETWORK "Enable TCP multiplayer through the libogc network stack" OFF)
# The Hollywood renders 640x480 (NTSC) / 640x528 (PAL) either way. This selects
# the initial value of the runtime aspect-ratio option, not a higher resolution.
option(WII_WIDESCREEN "Default the runtime aspect ratio to 16:9" OFF)
# Diagnostic verbosity shared by every target. See src/platform/Log.h.
# Level 2 remains opt-in because the SD stream is flushed synchronously and its
# runtime diagnostics inspect allocator/resource state.
set(MC_LOG_LEVEL "0" CACHE STRING "Unified diagnostic verbosity: 0=off, 1=info, 2=debug, 3=trace")
set_property(CACHE MC_LOG_LEVEL PROPERTY STRINGS 0 1 2 3)
# What to show after std::bad_alloc. OFF frees the failed world and returns to
# the main menu; ON keeps the diagnostic GuiErrorScreen visible. This used to be
# passed by the batch files but never reached the compiler, so both builds
# behaved as OFF.
option(WII_OOM_ERROR_SCREEN "Show the detailed in-game OOM diagnostic screen" ON)
# The EFB->XFB deflicker filter. ON matches what a Wii game normally does and is
# what a 480i TV needs -- Minecraft terrain is wall-to-wall high-contrast
# horizontal edges, which is exactly what interlaced flicker feeds on. OFF is
# sharper and costs slightly less copy bandwidth per frame, and is only sensible
# on a progressive display. A/B it on the actual TV; Dolphin will not tell you.
option(WII_DEFLICKER "Apply the EFB->XFB vertical deflicker filter" ON)
# Upload textures as 16-bit GX_TF_RGB5A3 instead of 32-bit GX_TF_RGBA8. Halves
# texture RAM and halves texel fetch bandwidth, which is real fill rate at
# 640x480 with Minecraft's overdraw. Costs 5 bits of colour on opaque texels and
# 4+3 on blended ones. OFF restores the exact previous image for an A/B.
option(WII_TEXTURE_RGB5A3 "Upload textures as 16-bit RGB5A3 rather than 32-bit RGBA8" ON)
# OFF for the same reason the PS2 Release linker keeps unwind sections: --gc-sections can discard C++ unwind
# data if the linker script lacks KEEP() on the exception tables, and the first
# thrown exception then jumps into garbage. Needs verifying against devkitPPC's
# rvl linker script before it can be turned on.
option(WII_GC_SECTIONS "Compile with -ffunction-sections/-fdata-sections and link with --gc-sections" OFF)
# Broadway has a hardware FPU, so PLATFORM_CONSOLE_LOW -- which is the *CPU*
# profile -- is the wrong default here: it swaps in heightmap terrain, float
# biome noise, trimmed Perlin octaves and a different bedrock RNG stream (i.e. it
# CHANGES the generated world), disables the light flood-fill, and drops
# options.txt load/save. All of that exists to work around an EE this console
# does not have. Left OFF; turn it ON only to A/B against the PS2 profile.
#
# The memory half of the old combined profile is WII_BOUNDED_WORLD below and is
# ON by default -- that is the part the Wii actually needed.
option(WII_CONSOLE_LOW "Opt into the PLATFORM_CONSOLE_LOW weak-CPU gameplay profile" OFF)
# Bound the resident world to a fixed memory budget: chunk cache radius, unload
# policy, preload radius, and the synchronous generation/decoration throttles,
# with the values in src/wii/WiiTuning.h. ON by default -- with it OFF the port
# takes the desktop assumptions (a 17x17 preload, a 21x21 unload radius,
# unthrottled synchronous generation) against ~60 MB of heap, which is what put
# it on the out-of-memory screen.
#
# OFF is an A/B switch, not a shipping configuration. Note that WII_CONSOLE_LOW=ON
# forces this on regardless: PlatformConfig.h #errors if the CPU profile is
# enabled without the memory profile it reads state from.
option(WII_BOUNDED_WORLD "Bound the resident world to the WiiTuning.h memory budget" ON)

# --- Source selection ---------------------------------------------------------
if(WII_BRINGUP)
    set(WII_SOURCES
        "${CMAKE_SOURCE_DIR}/src/wii/tools/WiiBringup.cpp"
        "${CMAKE_SOURCE_DIR}/src/wii/system/WiiConsole.cpp"
    )
    message(STATUS "Wii build: BRINGUP (toolchain smoke test only)")
else()
    mcbeta_collect_platform_sources(WII_SOURCES wii)

    # Wii stores stats locally, so the desktop synchronizer/JSON/MD5 stack is
    # unreachable and must not enter the target.
    mcbeta_exclude_remote_stats_sources(WII_SOURCES)

    # The Wii audio backend uses stb_vorbis without pulling the desktop SDL
    # backend into the target.
    list(APPEND WII_SOURCES "${CMAKE_SOURCE_DIR}/src/pc/external/stb_vorbis.cpp")

    # JavaNetwork.cpp is the SDL_net desktop backend. The Wii implementation
    # lives entirely in src/wii when networking is enabled.
    if(WII_ENABLE_NETWORK)
        mcbeta_exclude_sources(WII_SOURCES "[/\\]java[/\\]JavaNetwork\\.cpp$")
    else()
        mcbeta_exclude_sources(WII_SOURCES "[/\\]wii[/\\](JavaNetwork_wii|WiiNetwork)\\.cpp$")
    endif()

    # The toolchain smoke-test main is a separate target shape.
    mcbeta_exclude_sources(WII_SOURCES "[/\\]wii[/\\]tools[/\\]WiiBringup\\.cpp$")
    mcbeta_select_platform_backends(WII_SOURCES WII GX_WII WII)
    message(STATUS "Wii build: FULL game sources")
endif()

# --- Target -------------------------------------------------------------------
add_executable(BetaPlusPlus ${WII_SOURCES})
set_target_properties(BetaPlusPlus PROPERTIES
    SUFFIX ".elf"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_compile_features(BetaPlusPlus PRIVATE cxx_std_17)

target_compile_options(BetaPlusPlus PRIVATE
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:MinSizeRel>:-Os>
	
    $<$<COMPILE_LANGUAGE:CXX>:-frtti>

    # Same numerically-neutral relaxations the PS2 build uses. MathHelper::sqrt_float
    # sits on the chunk-build and entity paths; -fno-math-errno is what lets it
    # inline to a single fsqrt instead of a libm call.
    -fno-math-errno
    -fno-trapping-math

    # Deliberately NOT added here:
    #   -mpaired                 enables 750CL paired-singles (2-wide float SIMD).
    #                            Real win, but it needs 8-byte-aligned float pairs
    #                            and hand-written intrinsics to pay off; revisit
    #                            with a profile, not as a blanket flag.
    #   -fsingle-precision-constant  would demote double literals globally and
    #                            change worldgen/entity precision. Unlike the PS2
    #                            there is no reason to want it: the 750CL does
    #                            doubles in hardware.
    #   -O3 / -funroll-loops     the Broadway has 32 KB of L1I; code growth wants
    #                            measuring per-TU rather than a blanket add.
    $<$<BOOL:${WII_GC_SECTIONS}>:-ffunction-sections>
    $<$<BOOL:${WII_GC_SECTIONS}>:-fdata-sections>
)

target_compile_definitions(BetaPlusPlus PRIVATE
    "HW_RVL"
    "WII_PLATFORM"
    "GEKKO"
    $<$<NOT:$<BOOL:${WII_ENABLE_NETWORK}>>:NO_NETWORK>
    $<$<BOOL:${WII_ENABLE_NETWORK}>:WII_ENABLE_NETWORK=1>
    $<$<BOOL:${WII_WIDESCREEN}>:WII_WIDESCREEN>
    $<$<BOOL:${WII_CONSOLE_LOW}>:PLATFORM_CONSOLE_LOW=1>
    # Only the OFF case needs a definition: PlatformConfig.h already defaults
    # PLATFORM_BOUNDED_WORLD to 1 for PLATFORM_WII, so ON is the guarded default
    # and this predefine exists purely to switch it back off for an A/B run.
    $<$<NOT:$<BOOL:${WII_BOUNDED_WORLD}>>:PLATFORM_BOUNDED_WORLD=0>
    # Both of these are read as 0/1 rather than as defined/undefined, because
    # they sit in #if (not #ifdef) tests inside gx_wii.cpp and RenderEngine.cpp.
    WII_DEFLICKER=$<BOOL:${WII_DEFLICKER}>
    WII_TEXTURE_RGB5A3=$<BOOL:${WII_TEXTURE_RGB5A3}>
    # A level, not a boolean, so it is passed through as-is rather than through
    # $<BOOL:>, which would collapse 2 to 1.
    MC_LOG_LEVEL=${MC_LOG_LEVEL}
    WII_OOM_ERROR_SCREEN=$<BOOL:${WII_OOM_ERROR_SCREEN}>
)
if(NOT WII_ENABLE_SOUND)
    target_compile_definitions(BetaPlusPlus PRIVATE "NO_SOUND")
endif()

target_include_directories(BetaPlusPlus PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/pc"
    "${CMAKE_SOURCE_DIR}/src/wii"
    "${CMAKE_SOURCE_DIR}/external/stb"
    "${LIBOGC}/include"
    "${WII_PORTLIBS}/include"
    "${PPC_PORTLIBS}/include"
)

target_link_directories(BetaPlusPlus PRIVATE
    "${LIBOGC}/lib/wii"
    "${WII_PORTLIBS}/lib"
    "${PPC_PORTLIBS}/lib"
)

# --- Link libraries -----------------------------------------------------------
# libogc is a set of static libraries with real ordering constraints: a library
# must be listed before the ones it depends on. The canonical devkitPro order is
#   <portlibs> -lwiikeyboard -lfat -lasnd -lwiiuse -lbte -logc -lm
# and CMake preserves the order of plain library names.
set(_WII_LIBS "")

# zlib comes from the ppc portlibs (package ppc-zlib), replacing external/zlib.
# Chunk/region decompression is not optional, so a missing portlib is a hard
# error in the full build, but must not block the bring-up smoke test.
if(NOT WII_BRINGUP)
    find_library(WII_ZLIB NAMES z
        HINTS "${WII_PORTLIBS}/lib" "${PPC_PORTLIBS}/lib"
        NO_CMAKE_FIND_ROOT_PATH)
    if(WII_ZLIB)
        message(STATUS "Wii build: zlib = ${WII_ZLIB}")
        list(APPEND _WII_LIBS z)
    else()
        message(FATAL_ERROR
            "Wii build: zlib not found in ${WII_PORTLIBS}/lib or ${PPC_PORTLIBS}/lib.\n"
            "Install it from the devkitPro MSYS2 shell with:  dkp-pacman -S ppc-zlib")
    endif()
    # USB keyboard. Lets src/wii/lwjgl/Keyboard_wii.cpp feed the existing
    # lwjgl::Keyboard queue with real key events instead of an on-screen keyboard.
    list(APPEND _WII_LIBS wiikeyboard)
endif()

list(APPEND _WII_LIBS fat)                                  # SD / USB via libfat
if(WII_ENABLE_SOUND)
    list(APPEND _WII_LIBS asnd)                             # DSP mixer
endif()
list(APPEND _WII_LIBS wiiuse bte)                           # Wiimote + Bluetooth
list(APPEND _WII_LIBS ogc m)                              # Core + libm, last

target_link_libraries(BetaPlusPlus ${_WII_LIBS})

target_link_options(BetaPlusPlus PRIVATE
    "-Wl,-Map,${CMAKE_BINARY_DIR}/BetaPlusPlus.map"
    $<$<CONFIG:Release>:-s>
    $<$<BOOL:${WII_GC_SECTIONS}>:-Wl,--gc-sections>
)

set_target_properties(BetaPlusPlus PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/wii"
)

# --- Post-link assertion: no thread-local storage -----------------------------
# `thread_local` compiles and links on devkitPPC and then silently corrupts
# memory: there is no PT_TLS segment, so every thread_local in the program
# aliases one word addressed off r2 -- which crt0 loads with _SDA2_BASE_, not a
# thread pointer -- and that word lands in live .eh_frame_hdr data. Diagnosing it
# from the symptom is expensive (it first surfaced as stb_image loading every
# texture upside down, i.e. a white main menu), and nothing warns about it, so
# check the linked ELF directly. Runs before elf2dol so the failure is immediate.
find_program(WII_READELF NAMES powerpc-eabi-readelf HINTS "${DEVKITPPC}/bin")
if(WII_READELF)
    add_custom_command(TARGET BetaPlusPlus POST_BUILD
        COMMAND ${CMAKE_COMMAND}
                -DREADELF=${WII_READELF}
                -DELF=$<TARGET_FILE:BetaPlusPlus>
                -P "${CMAKE_CURRENT_LIST_DIR}/wii_check_no_tls.cmake"
        COMMENT "Checking BetaPlusPlus.elf for thread-local storage (unusable on devkitPPC)"
        VERBATIM
    )
else()
    message(WARNING
        "Wii build: powerpc-eabi-readelf not found under ${DEVKITPPC}/bin; the "
        "thread-local-storage check will be skipped. See cmake/wii_check_no_tls.cmake.")
endif()

# --- Packaging ----------------------------------------------------------------
# bin/wii/sd/ is shaped like the root of an SD card, so deploying is a plain copy.
# The name is historical: the same tree works verbatim on a USB drive, because
# the game resolves its install directory at runtime instead of hardcoding a
# device (src/wii/system/WiiEarlyStorage.cpp, resolveAppDir).
set(WII_SD_ROOT "${CMAKE_SOURCE_DIR}/bin/wii/sd")
set(WII_APP_DIR "${WII_SD_ROOT}/apps/BetaPlusPlus")
file(MAKE_DIRECTORY "${WII_APP_DIR}")

# The Homebrew Channel reads this to name and describe the entry.
file(WRITE "${WII_APP_DIR}/meta.xml"
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<app version=\"1\">
    <name>BetaPlusPlus</name>
    <coder>OptiJuegos</coder>
    <version>${PROJECT_VERSION}</version>
    <short_description>Minecraft Beta 1.7.3 C++ port</short_description>
    <long_description>Minecraft Beta 1.7.3 ported to C++ and to the Nintendo Wii.
Install to apps/BetaPlusPlus/ on an SD card or a USB drive; both work.
Game data is read from that folder's data/ and worlds are saved there.</long_description>
    <no_ios_reload/>
</app>
")

# elf2dol turns the ELF into the flat DOL the Homebrew Channel boots.
find_program(WII_ELF2DOL NAMES elf2dol HINTS "${DEVKITPRO}/tools/bin")
if(WII_ELF2DOL)
    add_custom_command(TARGET BetaPlusPlus POST_BUILD
        COMMAND "${WII_ELF2DOL}" "$<TARGET_FILE:BetaPlusPlus>" "${WII_APP_DIR}/boot.dol"
        COMMENT "elf2dol: ${WII_APP_DIR}/boot.dol"
        VERBATIM
    )
else()
    message(WARNING
        "Wii build: elf2dol not found under ${DEVKITPRO}/tools/bin; no boot.dol "
        "will be produced. Dolphin can still load bin/wii/BetaPlusPlus.elf directly.")
endif()

# Staging data/ is a few thousand file copies, far slower than the link itself,
# so it is a separate target instead of a POST_BUILD step. Run it once, and again
# whenever the assets change:
#     cmake --build build/wii-release --target wii-data
add_custom_target(wii-data
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/data/assets" "${WII_APP_DIR}/data/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/data/resources" "${WII_APP_DIR}/data/resources"
    COMMENT "Staging data/ into ${WII_APP_DIR}/data"
    VERBATIM
)
