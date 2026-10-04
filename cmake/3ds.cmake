# 3ds.cmake — Nintendo 3DS (ARM11 + PICA200) build branch for OptiCraft.
#
# Included from the top of CMakeLists.txt when cmake/3ds_toolchain.cmake is
# active (it sets NINTENDO_3DS), which then return()s so none of the desktop
# SDL2/glad/OpenGL configuration runs. Structured to mirror cmake/wii.cmake,
# the closest existing port (small devkit console, SD storage, devkit toolchain).
#
# Two build shapes are available through presets:
#
#   3ds-release / 3ds-debug
#       Build the full game. This is the normal shipping/development path.
#       Requires the 16 platform backends (RenderAPI_CTR_3DS, SoundManager_3DS,
#       ...) to exist; a missing one is a configure-time FATAL_ERROR, which is
#       the intended guard while the port is being brought up.
#
#   3ds-bringup
#       Builds only the toolchain/hardware smoke test (src/3ds/tools/DsBringup).
#       Proves devkitARM/libctru link and boot without pulling game code.
#
# Output: bin/3ds/OptiCraft.elf + OptiCraft.3dsx (+ OptiCraft.smdh), and an
# SD-card-shaped tree under bin/3ds/sd/ for the runtime data/ folder. Game data
# is read from sd:/opticraft/ at runtime (wired up by Resources_3DS in Phase 1).
# The .cia is NOT a cmake target: `build 3ds.bat` and the CI job both invoke
# makerom directly against resources/3ds_cia.rsf, right after this build, so
# the manifest stays one checked-in file instead of a generated one.

cmake_minimum_required(VERSION 3.21)

include(${CMAKE_SOURCE_DIR}/cmake/SourceSelection.cmake)

# --- 3DS feature options ------------------------------------------------------
option(3DS_BRINGUP "Build only the 3DS toolchain smoke test instead of the game" OFF)
option(3DS_ENABLE_SOUND "Enable the ndsp sound backend" ON)
option(3DS_ENABLE_NETWORK "Enable TCP multiplayer through the libctru soc:U socket service (OFF keeps the NO_NETWORK stubs)" ON)
# The New 3DS doubles RAM (124 MB vs 64 MB for apps) and triples the CPU clock;
# Old 3DS is the floor the port must stay playable on, so the CPU gameplay
# profile defaults OFF exactly like the Wii (whose Broadway has a real FPU too --
# the ARM11 has VFPv2 in hardware; see cmake/wii.cmake for why this is the *CPU*
# half of the old combined profile).
option(3DS_CONSOLE_LOW "Opt into the PLATFORM_CONSOLE_LOW weak-CPU gameplay profile" OFF)
# Memory half: bound the resident world to a fixed budget. ON by default --
# with it OFF the port takes desktop preload/unload radii against ~64 MB of
# application heap on an Old 3DS, which is an out-of-memory screen waiting to
# happen. PlatformConfig.h wires PLATFORM_3DS to this in Phase 1; the predefine
# below only exists for the OFF case (same pattern as WII_BOUNDED_WORLD).
option(3DS_BOUNDED_WORLD "Bound the resident world to a fixed memory budget" ON)
# 3dsx.specs already passes --gc-sections to the linker, so this option only
# adds the *granularity* it needs to remove anything smaller than a .o:
# -ffunction-sections/-fdata-sections (what devkitPro's own template passes).
# Default ON for size -- the .3dsx is copied into application RAM, so dead code
# costs memory twice over. Turn OFF only if C++ unwinding (.ARM.exidx) ever
# misbehaves under gc; the link itself stays gc'd either way (specs, not us).
option(3DS_GC_SECTIONS "Compile with -ffunction-sections/-fdata-sections (the linker's --gc-sections comes from 3dsx.specs regardless)" ON)
# Diagnostic verbosity shared by every target. See src/platform/Log.h.
set(MC_LOG_LEVEL "0" CACHE STRING "Unified diagnostic verbosity: 0=off, 1=info, 2=debug, 3=trace")
set_property(CACHE MC_LOG_LEVEL PROPERTY STRINGS 0 1 2 3)

# --- Source selection ---------------------------------------------------------
if(3DS_BRINGUP)
    set(3DS_SOURCES
        "${CMAKE_SOURCE_DIR}/src/3ds/tools/DsBringup.cpp"
    )
    message(STATUS "3DS build: BRINGUP (toolchain smoke test only)")
else()
    mcbeta_collect_platform_sources(3DS_SOURCES 3ds)
    set(3DS_MINIZIP_SOURCES
        "${CMAKE_SOURCE_DIR}/external/zlib/contrib/minizip/ioapi.c"
        "${CMAKE_SOURCE_DIR}/external/zlib/contrib/minizip/unzip.c"
        # The zip WRITER half of minizip, for the QR download's install step:
        # a pack downloaded in Java orientation is converted to this
        # console's disk convention (src/3ds/assets/DsAssetConvert.cpp)
        # before it lands in texturepacks/, which means rewriting the
        # archive with the flipped PNGs -- the runtime counterpart of
        # scripts/texturepack_flip_3ds.py.
        "${CMAKE_SOURCE_DIR}/external/zlib/contrib/minizip/zip.c"
    )
    list(APPEND 3DS_SOURCES ${3DS_MINIZIP_SOURCES})

    # devkitARM/newlib does not expose fopen64/ftello64/fseeko64, same as the
    # other consoles. Worlds are far below the 32-bit stdio limit, so keep this
    # compatibility define local to minizip instead of changing Minecraft's
    # file ABI globally.
    set_source_files_properties(${3DS_MINIZIP_SOURCES}
        PROPERTIES COMPILE_DEFINITIONS USE_FILE32API
    )

    # minizip sits on zlib. PS2 borrows it from ${PS2SDK}/ports and the Wii from
    # the ppc-zlib portlib, but devkitPro ships no zlib for the 3DS -- adding a
    # pacman package for a library already vendored in external/ would be a new
    # install step for every developer and for the CI container. Build the
    # vendored copy straight into the target instead, the same way the minizip
    # sources above are wired.
    #
    # The list mirrors ZLIB_SRCS in external/zlib/CMakeLists.txt (vendored and
    # frozen, so it cannot drift on its own) minus the gz* sources: that is
    # zlib's own stdio-over-descriptor gzip front end, which wants lseek/read/
    # write from <unistd.h> through a HAVE_UNISTD_H that only ./configure
    # supplies. Nothing in the game calls gzopen/gzread/gzwrite/gzclose (they
    # are the one zlib entry point minizip does not use), so leaving them out
    # costs nothing and keeps the compile configuration-free.
    set(3DS_ZLIB_SOURCES "")
    foreach(_3ds_zlib_src
        adler32 compress crc32 deflate
        inflate infback inftrees inffast trees uncompr zutil)
        list(APPEND 3DS_ZLIB_SOURCES "${CMAKE_SOURCE_DIR}/external/zlib/${_3ds_zlib_src}.c")
    endforeach()
    list(APPEND 3DS_SOURCES ${3DS_ZLIB_SOURCES})

    # quirc (external/quirc, ISC license) -- the QR decoder behind the
    # "Descarga QR" screen (src/3ds/qr): the 3DS camera hands it grayscale
    # frames and it returns the scanned URL. Plain C89 with no dependencies
    # and no floating point, so it compiles into this target as-is. Only the
    # 3DS compiles it: it is the only platform with a camera to scan codes
    # with, so a desktop or console link never drags it in.
    set(3DS_QUIRC_SOURCES
        "${CMAKE_SOURCE_DIR}/external/quirc/quirc.c"
        "${CMAKE_SOURCE_DIR}/external/quirc/decode.c"
        "${CMAKE_SOURCE_DIR}/external/quirc/identify.c"
        "${CMAKE_SOURCE_DIR}/external/quirc/version_db.c"
    )
    list(APPEND 3DS_SOURCES ${3DS_QUIRC_SOURCES})

    # zconf.h is deliberately absent from the source tree -- zlib's own
    # CMakeLists generates it into the build tree -- so regenerate it here
    # instead of putting external/zlib on the include path blind.
    set(3DS_ZLIB_GEN_DIR "${CMAKE_BINARY_DIR}/external/zlib")
    file(MAKE_DIRECTORY "${3DS_ZLIB_GEN_DIR}")
    configure_file("${CMAKE_SOURCE_DIR}/external/zlib/zconf.h.cmakein"
                   "${3DS_ZLIB_GEN_DIR}/zconf.h" @ONLY)

    # The 3DS audio backend consumes the PS2's ADP assets (SPU2-ADPCM,
    # adpenc output -- see src/platform/audio/SoundManager_3DS.cpp), decoded
    # with the portable software decoder the PS2 streamer uses. That single
    # file is listed here explicitly: it lives under src/ps2/audio but is
    # pure C++ with no PS2 SDK dependency -- the same cross-tree listing
    # shape stb_vorbis.cpp had here before the backend switched from OGG to
    # ADP (with that switch, stb_vorbis and the desktop OGG tree leave this
    # target entirely).
    list(APPEND 3DS_SOURCES "${CMAKE_SOURCE_DIR}/src/ps2/audio/Ps2AdpcmStreamDecoder.cpp")

    # Wii stores stats locally, so the desktop synchronizer/JSON/MD5 stack is
    # unreachable and must not enter the target. Same on 3DS (saves are on SD).
    mcbeta_exclude_remote_stats_sources(3DS_SOURCES)

    # JavaNetwork.cpp is the SDL_net desktop backend. With networking enabled
    # (the default) the implementation lives in src/3ds/JavaNetwork_3ds.cpp +
    # DsNetwork.cpp, mirroring cmake/wii.cmake; with it OFF the desktop file
    # stays compiled behind its NO_NETWORK stubs.
    if(3DS_ENABLE_NETWORK)
        mcbeta_exclude_sources(3DS_SOURCES "[/\\]java[/\\]JavaNetwork\\.cpp$")
    else()
        mcbeta_exclude_sources(3DS_SOURCES "[/\\]3ds[/\\](JavaNetwork_3ds|DsNetwork)\\.cpp$")
    endif()

    # The toolchain smoke-test main is a separate target shape.
    mcbeta_exclude_sources(3DS_SOURCES "[/\\]3ds[/\\]tools[/\\]DsBringup\\.cpp$")
    mcbeta_select_platform_backends(3DS_SOURCES 3DS CTR_3DS 3DS)

    # --- PICA200 vertex shader (picasso) -----------------------------------
    # src/3ds/render/DsShader.v.pica assembles at build time into a SHBIN plus
    # two headers in the build dir: picasso's own -h output (the uniform
    # register defines DsRender.cpp reads) and the embedded word array
    # cmake/DsShaderEmbed.cmake writes -- the CMake-side stand-in for the
    # bin2s-plus-assembler bridge the devkitPro makefiles use. Both are listed
    # as target sources so the custom commands run before anything that
    # includes them, and a missing picasso is a hard configure error rather
    # than a confusing missing-header compile error (it ships in the same
    # 3ds-dev package as smdhtool/3dsxtool).
    find_program(3DS_PICASSO NAMES picasso HINTS "${DEVKITPRO}/tools/bin")
    if(NOT 3DS_PICASSO)
        message(FATAL_ERROR
            "3DS build: picasso not found under ${DEVKITPRO}/tools/bin.\n"
            "It assembles the PICA200 vertex shader "
            "(src/3ds/render/DsShader.v.pica) and ships in the 3ds-dev\n"
            "package together with smdhtool/3dsxtool:  dkp-pacman -S 3ds-dev")
    endif()
    set(3DS_SHADER_PICA "${CMAKE_SOURCE_DIR}/src/3ds/render/DsShader.v.pica")
    set(3DS_SHADER_BIN  "${CMAKE_BINARY_DIR}/DsShader_shbin.bin")
    set(3DS_SHADER_HDR  "${CMAKE_BINARY_DIR}/DsShader_shbin.h")
    set(3DS_SHADER_DATA "${CMAKE_BINARY_DIR}/DsShader_shbin_data.h")
    add_custom_command(
        OUTPUT "${3DS_SHADER_BIN}" "${3DS_SHADER_HDR}"
        COMMAND "${3DS_PICASSO}" -o "${3DS_SHADER_BIN}" -h "${3DS_SHADER_HDR}"
                "${3DS_SHADER_PICA}"
        DEPENDS "${3DS_SHADER_PICA}"
        COMMENT "Assembling PICA200 vertex shader (picasso)"
        VERBATIM)
    add_custom_command(
        OUTPUT "${3DS_SHADER_DATA}"
        COMMAND "${CMAKE_COMMAND}"
                "-DDsShaderEmbed_INPUT=${3DS_SHADER_BIN}"
                "-DDsShaderEmbed_OUTPUT=${3DS_SHADER_DATA}"
                "-DDsShaderEmbed_SYMBOL=DsShader_shbin"
                -P "${CMAKE_SOURCE_DIR}/cmake/DsShaderEmbed.cmake"
        DEPENDS "${3DS_SHADER_BIN}"
        COMMENT "Embedding PICA200 vertex shader"
        VERBATIM)
    list(APPEND 3DS_SOURCES "${3DS_SHADER_HDR}" "${3DS_SHADER_DATA}")

    message(STATUS "3DS build: FULL game sources")
endif()

# --- Target -------------------------------------------------------------------
add_executable(OptiCraft ${3DS_SOURCES})
set_target_properties(OptiCraft PROPERTIES
    SUFFIX ".elf"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_compile_features(OptiCraft PRIVATE cxx_std_17)

target_compile_options(OptiCraft PRIVATE
    # Same profile devkitPro's own template uses; the game's hot loops want -O2
    # on Release and the Wii parity mapping for MinSizeRel.
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:MinSizeRel>:-Os>

    # devkitPro's template builds C++ with -fno-rtti -fno-exceptions; this
    # codebase needs RTTI (dynamic_cast in the shared code) and the exception
    # machinery. Exceptions stay at the GCC default (on) -- nothing disables
    # them, so no flag is passed here; RTTI is passed explicitly because the
    # Wii build does and because -frtti is not the first thing one expects on
    # an embedded target.
    $<$<COMPILE_LANGUAGE:CXX>:-frtti>

    # Numerically-neutral relaxations shared with the PS2/Wii builds:
    # -fno-math-errno is what lets MathHelper::sqrt_float inline to a single
    # fsqrt instead of a libm call (it sits on the chunk-build/entity paths).
    -fno-math-errno
    -fno-trapping-math

    # Section granularity is ON by default (see the 3DS_GC_SECTIONS option):
    # 3dsx.specs always links with --gc-sections, so without these flags it can
    # only reclaim whole translation units. The Wii build deliberately ships
    # without section GC (its libogc link does not force it); here the forcing
    # is in the specs file and out of our hands -- these flags just decide how
    # finely the always-on gc can cut.
    $<$<BOOL:${3DS_GC_SECTIONS}>:-ffunction-sections>
    $<$<BOOL:${3DS_GC_SECTIONS}>:-fdata-sections>
)

target_compile_definitions(OptiCraft PRIVATE
    # CTR, not 3DS: a preprocessor macro cannot start with a digit (-D3DS_...
    # is a hard "macro names must be identifiers" error), and CTR is the
    # console's own codename anyway (libctru, the CTR_3DS render backend).
    # PlatformConfig.h maps CTR_PLATFORM -> PLATFORM_3DS in Phase 1, the same
    # way it maps PS2_PLATFORM -> PLATFORM_PS2 today.
    "CTR_PLATFORM"
    $<$<NOT:$<BOOL:${3DS_ENABLE_NETWORK}>>:NO_NETWORK>
    $<$<BOOL:${3DS_ENABLE_NETWORK}>:CTR_ENABLE_NETWORK=1>
    $<$<BOOL:${3DS_CONSOLE_LOW}>:PLATFORM_CONSOLE_LOW=1>
    # Only the OFF case needs a definition: PlatformConfig.h defaults
    # PLATFORM_BOUNDED_WORLD for the 3DS profile (Phase 1 wiring), so ON is the
    # guarded default and this predefine exists purely to A/B it back off.
    $<$<NOT:$<BOOL:${3DS_BOUNDED_WORLD}>>:PLATFORM_BOUNDED_WORLD=0>
    # A level, not a boolean, so it is passed through as-is rather than through
    # $<BOOL:>, which would collapse 2 to 1.
    MC_LOG_LEVEL=${MC_LOG_LEVEL}
)
if(NOT 3DS_ENABLE_SOUND)
    target_compile_definitions(OptiCraft PRIVATE "NO_SOUND")
endif()

target_include_directories(OptiCraft PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/pc"
    "${CMAKE_SOURCE_DIR}/src/3ds"
    # Game code does bare #include "Minecraft.h" / "Tessellator.h" into
    # src/net/minecraft/src. CI injects this via CMAKE_CXX_FLAGS on other
    # platforms; add it here directly (as cmake/ps2.cmake does) so a local
    # configure works without extra flags.
    "${CMAKE_SOURCE_DIR}/src/net/minecraft/src"
    "${CMAKE_SOURCE_DIR}/external/stb"
    "${CMAKE_SOURCE_DIR}/external/zlib/contrib/minizip"
    # quirc: vendored QR decoder (see the 3DS_QUIRC_SOURCES block above).
    "${CMAKE_SOURCE_DIR}/external/quirc"
    # zlib itself (see the 3DS_ZLIB_SOURCES block: source dir for zlib.h,
    # build dir for the generated zconf.h).
    "${CMAKE_SOURCE_DIR}/external/zlib"
    "${CMAKE_BINARY_DIR}/external/zlib"
    # Generated shader headers: DsShader_shbin.h (picasso) and
    # DsShader_shbin_data.h (cmake/DsShaderEmbed.cmake). See the picasso
    # section above the target.
    "${CMAKE_BINARY_DIR}"
    "${LIBCTRU}/include"
    "${THREEDS_PORTLIBS}/include"
)

target_link_directories(OptiCraft PRIVATE
    "${LIBCTRU}/lib"
    "${THREEDS_PORTLIBS}/lib"
)

# --- Link libraries -----------------------------------------------------------
# Canonical devkitPro order (3ds-examples): libs that depend on others first,
# -lctru -lm last. CMake preserves the order of plain library names.
set(_3DS_LIBS "")

if(NOT 3DS_BRINGUP)
    # citro2d sits on top of citro3d; both live wherever the 3ds-dev packages
    # installed them (libctru/lib on current devkitPro, worth probing
    # portlibs/3ds too since older layouts differed).
    find_library(_3DS_CITRO2D NAMES citro2d
        HINTS "${LIBCTRU}/lib" "${THREEDS_PORTLIBS}/lib"
        NO_CMAKE_FIND_ROOT_PATH)
    find_library(_3DS_CITRO3D NAMES citro3d
        HINTS "${LIBCTRU}/lib" "${THREEDS_PORTLIBS}/lib"
        NO_CMAKE_FIND_ROOT_PATH)
    if(NOT _3DS_CITRO2D OR NOT _3DS_CITRO3D)
        message(FATAL_ERROR
            "3DS build: citro2d/citro3d not found under ${LIBCTRU}/lib or "
            "${THREEDS_PORTLIBS}/lib.\n"
            "Install them from the devkitPro shell with:  dkp-pacman -S 3ds-dev")
    endif()
    message(STATUS "3DS build: citro2d = ${_3DS_CITRO2D}")
    message(STATUS "3DS build: citro3d = ${_3DS_CITRO3D}")
    list(APPEND _3DS_LIBS citro2d citro3d)

    # libcurl + mbedTLS (3ds-curl portlib package): the QR downloader
    # (src/3ds/qr/DsHttpDownload.cpp) needs TLS 1.2+ and redirect following,
    # neither of which the system http:C/ssl:C stack provides. zlib comes
    # from the same portlibs install (3ds-zlib is a 3ds-curl dependency) --
    # the vendored zlib sources above build the game's SELF, this -lz is
    # only libcurl's own reference and they never mix (different headers
    # would be the hazard; kMaxDownloadBytes-level code only).
    foreach(_3ds_netlib IN ITEMS curl mbedtls mbedx509 mbedcrypto z)
        find_library(_3DS_NETLIB_${_3ds_netlib} NAMES ${_3ds_netlib}
            HINTS "${THREEDS_PORTLIBS}/lib"
            NO_CMAKE_FIND_ROOT_PATH)
        if(NOT _3DS_NETLIB_${_3ds_netlib})
            message(FATAL_ERROR
                "3DS build: lib${_3ds_netlib}.a not found under ${THREEDS_PORTLIBS}/lib.\n"
                "The QR downloader links libcurl + mbedTLS; install them from the\n"
                "devkitPro shell with:  dkp-pacman -S 3ds-curl")
        endif()
        list(APPEND _3DS_LIBS ${_3ds_netlib})
    endforeach()
endif()

list(APPEND _3DS_LIBS ctru m)                                # libctru + libm, last

target_link_libraries(OptiCraft ${_3DS_LIBS})

# -specs=3dsx.specs is what pulls in libctru's crt0/startup, the 3dsx.ld
# script, --gc-sections and --emit-relocs (see the file; it is the single
# non-obvious flag on the link line -- devkitPro's 3ds-examples template
# carries exactly this one).
#
# No -s/--strip here, unlike the Wii Release link: --emit-relocs exists so that
# 3dsxtool can read the ELF's relocation entries and rebuild them as the .3dsx
# load-time relo table, and `strip` discards relocation sections from
# executables. Stripping would produce an ELF that links and then dies in
# 3dsxtool (or, worse, a .3dsx that cannot be loaded). The unstripped .elf costs
# nothing at runtime: only what 3dsxtool copies (loadable segments) is loaded.
target_link_options(OptiCraft PRIVATE
    "-specs=3dsx.specs"
    "-Wl,-Map,${CMAKE_BINARY_DIR}/OptiCraft.map"
)

set_target_properties(OptiCraft PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/3ds"
)

# --- Packaging: .smdh + .3dsx -------------------------------------------------
# 3dsxtool/smdhtool ship in $DEVKITPRO/tools/bin (part of the 3ds-dev group).
# Missing tools degrade to a WARNING exactly like the Wii's elf2dol: the .elf
# still exists and is bootable from an emulator, the .3dsx just will not.
find_program(3DS_SMDHTOOL NAMES smdhtool HINTS "${DEVKITPRO}/tools/bin")
find_program(3DSXTOOL    NAMES 3dsxtool HINTS "${DEVKITPRO}/tools/bin")

# The title's cover, as seen on the HOME menu, in Azahar's game list and on the
# installed .cia (`build 3ds.bat` and CI pass this same .smdh to makerom's
# -icon). The project's own
# art wins; libctru's generic cube is only the fallback, so a clean clone still
# builds. resources/ is tracked by git, assets/ is not -- hence this path.
#
# The glob rather than EXISTS is deliberate: resources/3ds_icon.png can land
# after this directory was first configured, and CONFIGURE_DEPENDS makes the
# next build re-run configure so it gets picked up without wiping the tree.
# smdhtool rejects anything that is not exactly 48x48 ("Icon size is
# incorrect"), so what lives here has to be the finished square, not the
# portrait box art.
file(GLOB _3DS_PROJECT_ICON CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/resources/3ds_icon.png")
if(_3DS_PROJECT_ICON)
    list(GET _3DS_PROJECT_ICON 0 _3DS_ICON)
    message(STATUS "3DS build: using project icon ${_3DS_ICON}")
else()
    set(_3DS_ICON "${LIBCTRU}/default_icon.png")
    if(NOT EXISTS "${_3DS_ICON}")
        # No default icon available (layout difference); 3dsxtool without an SMDH
        # still produces a bootable .3dsx, just one without an icon/metadata.
        set(_3DS_ICON "")
        message(WARNING "3DS build: no icon at resources/3ds_icon.png or ${LIBCTRU}/default_icon.png")
    else()
        message(STATUS "3DS build: no resources/3ds_icon.png; using the libctru default cube")
    endif()
endif()

if(3DS_SMDHTOOL AND 3DSXTOOL AND _3DS_ICON)
    add_custom_command(TARGET OptiCraft POST_BUILD
        COMMAND "${3DS_SMDHTOOL}" --create
                "OptiCraft Heritage"
                "Minecraft 1.2.5 clean-room C++ port"
                "OptiJuegos"
                "${_3DS_ICON}"
                "${CMAKE_SOURCE_DIR}/bin/3ds/OptiCraft.smdh"
        COMMAND "${3DSXTOOL}"
                "$<TARGET_FILE:OptiCraft>"
                "${CMAKE_SOURCE_DIR}/bin/3ds/OptiCraft.3dsx"
                "--smdh=${CMAKE_SOURCE_DIR}/bin/3ds/OptiCraft.smdh"
        COMMENT "Packaging bin/3ds/OptiCraft.3dsx"
        VERBATIM
    )
else()
    message(WARNING
        "3DS build: smdhtool/3dsxtool not found under ${DEVKITPRO}/tools/bin; "
        "no .3dsx will be produced. An emulator can still load bin/3ds/OptiCraft.elf.")
endif()

# --- SD tree ------------------------------------------------------------------
# bin/3ds/sd/ is shaped like the root of an SD card, so deploying is a plain
# copy of its contents. The runtime base directory is sd:/opticraft/ (decided in
# the port plan; Resources_3DS.cpp resolves it).
#
# Staging data/ is a few thousand file copies, far slower than the link itself,
# so it is a separate target instead of a POST_BUILD step:
#     cmake --build build/3ds-release --target 3ds-data
set(3DS_SD_ROOT "${CMAKE_SOURCE_DIR}/bin/3ds/sd")
set(3DS_APP_DIR "${3DS_SD_ROOT}/opticraft")
file(MAKE_DIRECTORY "${3DS_APP_DIR}")

add_custom_target(3ds-data
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/data/assets" "${3DS_APP_DIR}/data/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/data/resources_ps2" "${3DS_APP_DIR}/data/resources"
    COMMENT "Staging data/ into ${3DS_APP_DIR}/data"
    VERBATIM
)
