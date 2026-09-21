# ps2.cmake — PlayStation 2 (Emotion Engine) build branch for OptiCraft.
#
# Included from the top of CMakeLists.txt when the EE toolchain is active
# (psdevwindows/ps2sdk/ps2dev.cmake defines PS2), which then return()s so none
# of the desktop SDL2/glad/OpenGL configuration runs. Derived from the
# reference CMakeListsPS2FOROTHERBUILD.txt (MCPE port) but adapted to Beta's
# source tree and target name.
#
# Entry point: main_ps2.cpp → Minecraft::start().
# PC lwjgl/ shims are replaced by src/ps2/lwjgl/ equivalents (no SDL).

cmake_minimum_required(VERSION 3.21)

include(${CMAKE_SOURCE_DIR}/cmake/SourceSelection.cmake)

set(MC_LOG_LEVEL "0" CACHE STRING "Unified diagnostic verbosity: 0=off, 1=info, 2=debug, 3=trace")
set_property(CACHE MC_LOG_LEVEL PROPERTY STRINGS 0 1 2 3)

# Both default in platform/Log.h; overridable here so the logger's own cost can
# be taken out of a measurement without editing the header. Every synced line
# costs an fflush/fclose/fopen on the storage device, which is the single most
# expensive thing a verbose build does -- and it makes the log sink a suspect in
# its own right when a run stops immediately after a trace line.
set(MC_LOG_SYNC_WRITES "" CACHE STRING "Commit every log line to the device: 1=on (console default), 0=off")
set(MC_LOG_COMMIT_EVERY "" CACHE STRING "Synced lines between file close/reopen; higher trades tail precision for speed")

# --- SDK roots (the preset exports PS2DEV/PS2SDK into the environment) --------
if(NOT DEFINED PS2DEV)
    if(DEFINED ENV{PS2DEV})
        set(PS2DEV "$ENV{PS2DEV}")
    else()
        set(PS2DEV "/usr/local/ps2dev")
    endif()
endif()
if(NOT DEFINED PS2SDK)
    if(DEFINED ENV{PS2SDK})
        set(PS2SDK "$ENV{PS2SDK}")
    else()
        set(PS2SDK "${PS2DEV}/ps2sdk")
    endif()
endif()
message(STATUS "PS2 build: PS2DEV=${PS2DEV}")
message(STATUS "PS2 build: PS2SDK=${PS2SDK}")

# --- PS2 feature options (mirrors the reference build) ------------------------
option(PS2_NTSC_MODE "Use runtime NTSC/PAL interlaced SD mode instead of 480P (recommended for real PS2/OPL)" ON)
option(PS2_ENABLE_VU1_TERRAIN "Build the experimental direct VIF1/VU1/XGKICK terrain path" OFF)
option(PS2_ENABLE_VU0_MESH_FINALIZE "Use asynchronous VIF0/VU0 micro mode for terrain mesh finalization" ON)
# ON: the UV (non-STQ) path interpolates texture coordinates affinely in
# screen space, so any surface at an angle — water planes, mob skins, items
# lying on the ground — visibly "swims"/warps as the camera moves (PS1 look),
# and near-camera geometry can smear neighbouring atlas tiles across a face.
# The STQ path is also faster: it batches 64 triangles per GIF packet.
option(PS2_ENABLE_PERSPECTIVE_TEXTURES "Use PS2 STQ perspective-correct texture mapping in the fast draw path" ON)
option(PS2_ENABLE_PSMT8 "Store game textures as 8-bit palettized PSMT8 + CT16 CLUT (halves texture VRAM/RAM)" ON)
option(PS2_RENDER_STATS "Enable verbose PS2 render statistics counters" OFF)
option(PS2_REMOTE_DEBUG "Enable hardware remote debugging through ps2link/ps2client" OFF)
option(PS2_ENABLE_SOUND "Enable PS2 audsrv ADPCM sound backend" ON)
option(PS2_ENABLE_NETWORK "Enable PS2 TCP multiplayer through PS2SDK ps2ip/SMAP" ON)

if(PS2_REMOTE_DEBUG AND CMAKE_BUILD_TYPE STREQUAL "Release")
    message(FATAL_ERROR "PS2_REMOTE_DEBUG requires a symbol-preserving build type; use the ps2-remote-debug preset")
endif()

# Release always uses function/data sections, section GC, symbol stripping and
# the exception-safe private linker script below. Debug deliberately keeps the
# ordinary PS2SDK layout and symbols.

# --- Source selection ---------------------------------------------------------
mcbeta_collect_platform_sources(PS2_SOURCES ps2)
set(PS2_MINIZIP_SOURCES
    "${CMAKE_SOURCE_DIR}/external/zlib/contrib/minizip/ioapi.c"
    "${CMAKE_SOURCE_DIR}/external/zlib/contrib/minizip/unzip.c"
)
list(APPEND PS2_SOURCES ${PS2_MINIZIP_SOURCES})

# PS2SDK/newlib does not expose the glibc large-file stdio aliases fopen64,
# ftello64 and fseeko64 used by minizip's default ioapi path. Tutorial saves are
# tiny compared with the 32-bit file API limit, so keep this compatibility
# define local to minizip instead of changing Minecraft's file ABI globally.
set_source_files_properties(${PS2_MINIZIP_SOURCES}
    PROPERTIES COMPILE_DEFINITIONS USE_FILE32API
)

# Console-local stats never instantiate the desktop/server synchronizer.
mcbeta_exclude_remote_stats_sources(PS2_SOURCES)

mcbeta_select_platform_backends(PS2_SOURCES PS2 GS_PS2 PS2)

# JavaNetwork.cpp is the desktop/fallback implementation. When networking is
# enabled on PS2, select the native ps2ip socket backend instead.
if(PS2_ENABLE_NETWORK)
    mcbeta_exclude_sources(PS2_SOURCES "[/\\]java[/\\]JavaNetwork\\.cpp$")
else()
    mcbeta_exclude_sources(PS2_SOURCES "[/\\]ps2[/\\]JavaNetwork_ps2\\.cpp$")
endif()

# VU microprograms use the same dvp-as tool. Keep discovery shared so enabling
# either backend does not duplicate toolchain probing. Both paths retain CPU/VU0
# macro fallbacks when the assembler is unavailable.
if(PS2_ENABLE_VU1_TERRAIN OR PS2_ENABLE_VU0_MESH_FINALIZE)
    find_program(PS2_DVP_AS NAMES dvp-as
        HINTS "${PS2DEV}/dvp/bin" "${PS2DEV}/bin")
endif()

set(PS2_VU1_TERRAIN_ACTIVE OFF)
if(PS2_ENABLE_VU1_TERRAIN)
    if(PS2_DVP_AS)
        set(PS2_VU1_TERRAIN_VSM "${CMAKE_SOURCE_DIR}/src/ps2/vu1/Ps2Vu1Terrain.vsm")
        set(PS2_VU1_TERRAIN_VO "${CMAKE_BINARY_DIR}/vu1/Ps2Vu1Terrain.vo")
        add_custom_command(
            OUTPUT "${PS2_VU1_TERRAIN_VO}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/vu1"
            COMMAND "${PS2_DVP_AS}" -o "${PS2_VU1_TERRAIN_VO}" "${PS2_VU1_TERRAIN_VSM}"
            DEPENDS "${PS2_VU1_TERRAIN_VSM}"
            COMMENT "Assembling PS2 direct terrain VU1 microprogram"
            VERBATIM
        )
        set_source_files_properties("${PS2_VU1_TERRAIN_VO}" PROPERTIES
            EXTERNAL_OBJECT TRUE
            GENERATED TRUE
        )
        list(APPEND PS2_SOURCES "${PS2_VU1_TERRAIN_VO}")
        set(PS2_VU1_TERRAIN_ACTIVE ON)
        message(STATUS "PS2 build: direct VU1 terrain backend enabled (${PS2_DVP_AS})")
    else()
        message(WARNING "PS2_ENABLE_VU1_TERRAIN requested but dvp-as was not found; using the native VU0 fallback")
    endif()
endif()

# Optional VU0 micro-mode mesh finalizer. The feature only accelerates numeric
# XYZ/UV packing; Ps2TerrainMesh keeps the scalar path as a per-batch fallback.
set(PS2_VU0_MESH_FINALIZE_ACTIVE OFF)
if(PS2_ENABLE_VU0_MESH_FINALIZE)
    if(PS2_DVP_AS)
        set(PS2_VU0_MESH_FINALIZE_VSM "${CMAKE_SOURCE_DIR}/src/ps2/vu0/Ps2Vu0MeshFinalize.vsm")
        set(PS2_VU0_MESH_FINALIZE_VO "${CMAKE_BINARY_DIR}/vu0/Ps2Vu0MeshFinalize.vo")
        add_custom_command(
            OUTPUT "${PS2_VU0_MESH_FINALIZE_VO}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/vu0"
            COMMAND "${PS2_DVP_AS}" -o "${PS2_VU0_MESH_FINALIZE_VO}" "${PS2_VU0_MESH_FINALIZE_VSM}"
            DEPENDS "${PS2_VU0_MESH_FINALIZE_VSM}"
            COMMENT "Assembling PS2 asynchronous VU0 mesh-finalize microprogram"
            VERBATIM
        )
        set_source_files_properties("${PS2_VU0_MESH_FINALIZE_VO}" PROPERTIES
            EXTERNAL_OBJECT TRUE
            GENERATED TRUE
        )
        list(APPEND PS2_SOURCES "${PS2_VU0_MESH_FINALIZE_VO}")
        set(PS2_VU0_MESH_FINALIZE_ACTIVE ON)
        message(STATUS "PS2 build: asynchronous VU0 mesh finalizer enabled (${PS2_DVP_AS})")
    else()
        message(WARNING "PS2_ENABLE_VU0_MESH_FINALIZE requested but dvp-as was not found; using CPU mesh packing")
    endif()
endif()

# Required PS2 compatibility shims. Keep these explicit so a stale CMake glob or
# an incremental build cannot silently omit them and then fail only at link time.
set(PS2_REQUIRED_SHIMS
    "${CMAKE_SOURCE_DIR}/src/ps2/java/File_ps2.cpp"
    "${CMAKE_SOURCE_DIR}/src/ps2/java/Runtime_ps2.cpp"
    # Supplies __atomic_exchange_4; -mno-llsc leaves it as an unresolved libcall.
    "${CMAKE_SOURCE_DIR}/src/ps2/system/Ps2Atomic.c"
)

# libps2ip can change .data ordering enough for PS2SDK libkernel's errno archive
# member to land only 2-byte aligned. errno is a 32-bit int, so provide a known-
# aligned application definition for networking builds on the EE.
if(PS2_ENABLE_NETWORK)
    set(_PS2_ALIGNED_ERRNO_SOURCE
        "${CMAKE_SOURCE_DIR}/src/ps2/system/Ps2AlignedErrno.c")
    list(APPEND PS2_REQUIRED_SHIMS "${_PS2_ALIGNED_ERRNO_SOURCE}")
    # Preserve the dedicated section and explicit alignment through the final link.
    set_source_files_properties("${_PS2_ALIGNED_ERRNO_SOURCE}"
        PROPERTIES COMPILE_OPTIONS "-fno-lto")
endif()

foreach(_ps2_shim IN LISTS PS2_REQUIRED_SHIMS)
    list(REMOVE_ITEM PS2_SOURCES "${_ps2_shim}")
    list(APPEND PS2_SOURCES "${_ps2_shim}")
endforeach()

message(STATUS "PS2 build: game sources (assets are read from data/ at runtime)")

# --- Linker script -------------------------------------------------------------
# ps2dev.cmake supplies the stock PS2SDK linkfile through the toolchain. The
# wrapper removes that default so this target can select either the untouched
# SDK script or an exception-safe private copy when section GC is active.
set(_PS2_SDK_LINKFILE "")
foreach(_ps2_linkfile_candidate
    "${PS2SDK}/ee/startup/linkfile"
    "${PS2SDK}/ee/startup/src/linkfile"
)
    if(EXISTS "${_ps2_linkfile_candidate}")
        set(_PS2_SDK_LINKFILE "${_ps2_linkfile_candidate}")
        break()
    endif()
endforeach()
if(NOT _PS2_SDK_LINKFILE)
    message(FATAL_ERROR "PS2 build: PS2SDK EE linkfile was not found under ${PS2SDK}/ee/startup")
endif()

set(_PS2_ACTIVE_LINKFILE "${_PS2_SDK_LINKFILE}")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    file(READ "${_PS2_SDK_LINKFILE}" _ps2_linkfile_text)
    set(_PS2_EXCEPTION_RULE ".gcc_except_table ALIGN(128): { *(.gcc_except_table) }")
    string(FIND "${_ps2_linkfile_text}" "${_PS2_EXCEPTION_RULE}" _ps2_exception_rule_pos)

    if(_ps2_exception_rule_pos EQUAL -1)
        string(FIND "${_ps2_linkfile_text}" "KEEP(*(.gcc_except_table.*))" _ps2_has_exception_keep)
        string(FIND "${_ps2_linkfile_text}" "KEEP(*(.eh_frame.*))" _ps2_has_eh_keep)
        if(_ps2_has_exception_keep EQUAL -1 OR _ps2_has_eh_keep EQUAL -1)
            message(FATAL_ERROR
                "PS2 build: refusing --gc-sections because the PS2SDK linkfile layout is unknown; "
                "C++ exception metadata cannot be preserved safely")
        endif()
    else()
        set(_PS2_EXCEPTION_RULE_SAFE
".gcc_except_table ALIGN(128): {
		KEEP(*(.gcc_except_table))
		KEEP(*(.gcc_except_table.*))
	}
	.eh_frame ALIGN(128): {
		KEEP(*(.eh_frame))
		KEEP(*(.eh_frame.*))
	}")
        string(REPLACE "${_PS2_EXCEPTION_RULE}" "${_PS2_EXCEPTION_RULE_SAFE}"
            _ps2_linkfile_text "${_ps2_linkfile_text}")
    endif()

    set(_PS2_ACTIVE_LINKFILE "${CMAKE_CURRENT_BINARY_DIR}/ps2_linkfile_gc.ld")
    file(WRITE "${_PS2_ACTIVE_LINKFILE}" "${_ps2_linkfile_text}")
    message(STATUS "PS2 build: section GC uses exception-safe linker script ${_PS2_ACTIVE_LINKFILE}")
endif()

# A stale build cache can preserve the PS2SDK's global startup linkfile even
# after the wrapper toolchain has moved linker-script selection to this target.
# Two complete scripts produce incoherent VMA/LMA placement, so stop during
# configuration instead of discovering it in a malformed or failed final link.
string(TOUPPER "${CMAKE_BUILD_TYPE}" _PS2_BUILD_TYPE_UPPER)
set(_PS2_EFFECTIVE_GLOBAL_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_${_PS2_BUILD_TYPE_UPPER}}")
string(REPLACE "\\" "/" _PS2_EFFECTIVE_GLOBAL_LINKER_FLAGS_NORMALIZED
    "${_PS2_EFFECTIVE_GLOBAL_LINKER_FLAGS}")
string(TOLOWER "${_PS2_EFFECTIVE_GLOBAL_LINKER_FLAGS_NORMALIZED}"
    _PS2_EFFECTIVE_GLOBAL_LINKER_FLAGS_NORMALIZED)
if(_PS2_EFFECTIVE_GLOBAL_LINKER_FLAGS_NORMALIZED MATCHES
        "(^|[ \t])-t[^ \t]*/ps2sdk/ee/startup/(src/)?linkfile($|[ \t])")
    message(FATAL_ERROR
        "PS2 build has both the stock PS2SDK linker script and ${_PS2_ACTIVE_LINKFILE}. "
        "Clear the stale CMake cache or fix cmake/ps2_toolchain.cmake.")
endif()

# --- Target -------------------------------------------------------------------
add_executable(OptiCraft ${PS2_SOURCES})
set_target_properties(OptiCraft PROPERTIES SUFFIX ".elf")

target_compile_features(OptiCraft PRIVATE cxx_std_17)

target_compile_options(OptiCraft PRIVATE
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:MinSizeRel>:-O2>
    $<$<BOOL:${PS2_REMOTE_DEBUG}>:-O2>
    $<$<BOOL:${PS2_REMOTE_DEBUG}>:-g>
    $<$<CONFIG:Release>:-fno-ident>
    $<$<CONFIG:Release>:-fno-asynchronous-unwind-tables>
    # GCC follows every integer div/mod with a `teq` that traps on a zero divisor.
    # Chunk and light math divide constantly, and the game never relies on that
    # trap (it would be an unrecoverable EE exception anyway), so drop it.
    -mno-check-zero-division

    # Numerically neutral float relaxations. Both only remove bookkeeping GCC
    # emits for guarantees the EE FPU does not provide in the first place (it has
    # no denormals and does not implement IEEE NaN/Inf semantics -- PCSX2 even
    # warns about the clamp mode), so neither changes a computed value:
    #   -fno-math-errno    stops treating sqrtf/fabsf etc. as able to set errno,
    #                      which is what lets them inline to a single instruction
    #                      instead of a libm call. MathHelper::sqrt_float is on
    #                      the chunk-build and entity paths.
    #   -fno-trapping-math drops the assumption that any FP op might trap, which
    #                      frees GCC to reorder and hoist float work.
    #
    # Deliberately NOT added here:
    #   -mgpopt / -G       small-data addressing needs $gp set up, and it is not:
    #                      the ELF loads with "gp address 00000000". It would
    #                      relocate globals through a register holding zero.
    #   -fsingle-precision-constant  would demote double literals globally, which
    #                      changes world generation and entity coordinate
    #                      precision. The targeted equivalent is converting the
    #                      Tessellator/RenderBlocks vertex math to float, which
    #                      stays inside the renderer.
    #   -O3 / -funroll-loops  both grow code, and the EE has only a 16KB I-cache;
    #                      these want measuring per-TU rather than a blanket add.
    -fno-math-errno
    -fno-trapping-math

    # Cache geometry of the Emotion Engine, which GCC otherwise guesses from a
    # generic MIPS desktop profile. These three params do not enable anything;
    # they only correct the model that -O2's loop blocking, unrolling and
    # software-prefetch heuristics already consult, so with the wrong numbers
    # those heuristics size their work for a machine that does not exist.
    #
    #   R5900: 16 KB I-cache and 8 KB D-cache, both 2-way, 64-byte lines,
    #          and no L2 at all. l2-cache-size=0 is the literal truth here,
    #          not a way of disabling something.
    # SHELL: is required, not decoration. Written as plain list items, CMake
    # sees six words, de-duplicates the three identical "--param" tokens and
    # emits `--param l1-cache-size=8 l1-cache-line-size=64 l2-cache-size=0`;
    # GCC then reads the last two as input filenames and the build dies with
    # "linker input file not found". SHELL: keeps each flag and its value
    # together as one unit and exempts it from de-duplication.
    "SHELL:--param l1-cache-size=8"
    "SHELL:--param l1-cache-line-size=64"
    "SHELL:--param l2-cache-size=0"
    $<$<CONFIG:Release>:-ffunction-sections>
    $<$<CONFIG:Release>:-fdata-sections>
)

target_compile_definitions(OptiCraft PRIVATE
    "_EE"
    "PS2_PLATFORM"
    "NO_EGL"
    $<$<NOT:$<BOOL:${PS2_ENABLE_NETWORK}>>:NO_NETWORK>
    $<$<BOOL:${PS2_ENABLE_NETWORK}>:PS2_ENABLE_NETWORK=1>
    $<$<BOOL:${PS2_NTSC_MODE}>:PS2_NTSC_MODE>
    $<$<BOOL:${PS2_VU1_TERRAIN_ACTIVE}>:PS2_ENABLE_VU1_TERRAIN>
    $<$<BOOL:${PS2_VU0_MESH_FINALIZE_ACTIVE}>:PS2_ENABLE_VU0_MESH_FINALIZE>
    $<$<BOOL:${PS2_ENABLE_PERSPECTIVE_TEXTURES}>:PS2_ENABLE_PERSPECTIVE_TEXTURES>
    $<$<BOOL:${PS2_ENABLE_PSMT8}>:PS2_ENABLE_PSMT8>
    $<$<BOOL:${PS2_RENDER_STATS}>:PS2_RENDER_STATS>
    $<$<BOOL:${PS2_REMOTE_DEBUG}>:PS2_REMOTE_DEBUG>
    MC_LOG_LEVEL=${MC_LOG_LEVEL}
)

# Values, not booleans, and 0 is a meaningful setting for both -- so they are
# emitted only when set, by an explicit emptiness test. A $<BOOL:> generator
# expression would swallow exactly the case worth testing.
if(NOT MC_LOG_SYNC_WRITES STREQUAL "")
    target_compile_definitions(OptiCraft PRIVATE MC_LOG_SYNC_WRITES=${MC_LOG_SYNC_WRITES})
endif()
if(NOT MC_LOG_COMMIT_EVERY STREQUAL "")
    target_compile_definitions(OptiCraft PRIVATE MC_LOG_COMMIT_EVERY=${MC_LOG_COMMIT_EVERY})
endif()
if(NOT PS2_ENABLE_SOUND)
    target_compile_definitions(OptiCraft PRIVATE "NO_SOUND")
endif()

target_include_directories(OptiCraft PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/pc"
    "${CMAKE_SOURCE_DIR}/src/ps2"
    "${CMAKE_SOURCE_DIR}/src/net/minecraft/src"
    "${CMAKE_SOURCE_DIR}/src/mods"
    "${CMAKE_SOURCE_DIR}/external/stb"
    "${CMAKE_SOURCE_DIR}/external/miniaudio"
    "${CMAKE_SOURCE_DIR}/external/zlib/contrib/minizip"
    "${PS2SDK}/ee/include"
    "${PS2SDK}/common/include"
    "${PS2SDK}/ports/include"
    "${PS2DEV}/gsKit/include"
)

target_link_directories(OptiCraft PRIVATE
    "${PS2SDK}/ee/lib"
    "${PS2SDK}/ports/lib"
    "${PS2DEV}/gsKit/lib"
)

target_link_libraries(OptiCraft
    gskit dmakit dma graph
    patches pad mc vux
    $<$<BOOL:${PS2_ENABLE_SOUND}>:audsrv>
    z
    $<$<BOOL:${PS2_ENABLE_NETWORK}>:ps2ip>
    $<$<BOOL:${PS2_ENABLE_NETWORK}>:netman>
    kernel c
)

if(PS2_ENABLE_NETWORK)
    # Make the application-owned aligned definition satisfy errno before libkernel.a
    # is scanned, and keep it alive when --gc-sections is enabled.
    target_link_options(OptiCraft PRIVATE "-Wl,--undefined=errno")
endif()

target_link_options(OptiCraft PRIVATE
    "-T${_PS2_ACTIVE_LINKFILE}"
    "-Wl,-Map,${CMAKE_BINARY_DIR}/OptiCraft.map"
    $<$<CONFIG:Release>:-s>
    $<$<CONFIG:Release>:-Wl,-zmax-page-size=128>
    $<$<CONFIG:Release>:-Wl,--gc-sections>

    # Link compatibility only, not application LTO. The bundled PS2SDK archives
    # contain slim LTO objects, so the GCC linker plugin must materialize them.
    # A single partition also avoids the Windows/MSYS lto-wrapper make/shell path.
    -flto=1
    -flto-partition=none
)

# Mirror the desktop build's predictable output location.
set_target_properties(OptiCraft PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/ps2"
)

# --- Post-link validation, size pass and packaging -----------------------------
# The -s link flag drops the symbol table but leaves non-loaded MIPS debug
# sections (.pdr, .mdebug, .comment) in the file (~400 KB). Validate the linked
# layout, strip those sections, validate the exact deployable file, and only then
# replace the staged USB ELF. A failed link or validator therefore cannot publish
# a stale or malformed new binary.
find_program(PS2_OBJCOPY NAMES mips64r5900el-ps2-elf-objcopy ee-objcopy
    HINTS "${PS2DEV}/ee/bin" "${PS2DEV}/bin")
find_program(PS2_READELF NAMES mips64r5900el-ps2-elf-readelf ee-readelf
    HINTS "${PS2DEV}/ee/bin" "${PS2DEV}/bin")

if(NOT PS2_READELF)
    message(FATAL_ERROR "PS2 build: readelf is required to validate the packaged ELF")
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT PS2_OBJCOPY)
    message(FATAL_ERROR "PS2 build: objcopy is required to strip and package a Release ELF")
endif()

set(PS2_USB_ROOT "${CMAKE_SOURCE_DIR}/bin/ps2/usb")
set(PS2_APP_DIR  "${PS2_USB_ROOT}/MCBETA")
set(_PS2_ELF_VALIDATOR "${CMAKE_SOURCE_DIR}/cmake/ps2_validate_elf.cmake")
set(_PS2_LINKED_SIZE_REPORT "${CMAKE_BINARY_DIR}/OptiCraft.linked-size.txt")
set(_PS2_PACKAGED_SIZE_REPORT "${CMAKE_BINARY_DIR}/OptiCraft.size.txt")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET OptiCraft POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            "-DPS2_READELF=${PS2_READELF}"
            "-DELF=$<TARGET_FILE:OptiCraft>"
            "-DREPORT_FILE=${_PS2_LINKED_SIZE_REPORT}"
            "-DLABEL=linked"
            -P "${_PS2_ELF_VALIDATOR}"
        COMMAND "${PS2_OBJCOPY}" --strip-all -R .pdr -R .mdebug -R .comment
            "$<TARGET_FILE:OptiCraft>"
        COMMAND ${CMAKE_COMMAND}
            "-DPS2_READELF=${PS2_READELF}"
            "-DELF=$<TARGET_FILE:OptiCraft>"
            "-DREPORT_FILE=${_PS2_PACKAGED_SIZE_REPORT}"
            "-DLABEL=packaged"
            -P "${_PS2_ELF_VALIDATOR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PS2_APP_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:OptiCraft>" "${PS2_APP_DIR}/OptiCraft.elf"
        COMMENT "Validating, stripping and packaging ${PS2_APP_DIR}/OptiCraft.elf"
        VERBATIM
    )
else()
    add_custom_command(TARGET OptiCraft POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            "-DPS2_READELF=${PS2_READELF}"
            "-DELF=$<TARGET_FILE:OptiCraft>"
            "-DREPORT_FILE=${_PS2_PACKAGED_SIZE_REPORT}"
            "-DLABEL=packaged-unstripped"
            -P "${_PS2_ELF_VALIDATOR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PS2_APP_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:OptiCraft>" "${PS2_APP_DIR}/OptiCraft.elf"
        COMMENT "Validating and packaging ${PS2_APP_DIR}/OptiCraft.elf"
        VERBATIM
    )
endif()

# Staging data/ is a few hundred file copies, slower than the link itself, so it
# is a separate target instead of a POST_BUILD step. Run it once, and again
# whenever the assets change:
#     cmake --build build/ps2-release --target ps2-data
#
# data/resources_ps2 is staged as data/resources because that is the name the
# asset keys use; data/resources itself (desktop OGG) is never copied -- the PS2
# can only play the converted ADPCM.
add_custom_target(ps2-data
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/data/assets"        "${PS2_APP_DIR}/data/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/data/startup"       "${PS2_APP_DIR}/data/startup"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/data/resources_ps2" "${PS2_APP_DIR}/data/resources"

    # Audio that is deliberately not shipped. This used to be about ELF size;
    # now the cost is SPU2 memory, which is 2 MB and has no eviction here --
    # ps2GetAdpcmSample() uploads a sample on first play and never unloads it.
    #
    #     newsound/ambient   1141 KB   cave ambience + rain/thunder beds
    #     sound/loops         ~2 MB    C418 ocean/cave/bird loops
    #
    # Neither is reachable often enough to be worth that budget:
    # World::updateBlocksAndPlayCaveSounds reseeds its counter to 6000-18000
    # ticks (5-15 minutes), and PS2_SKIP_RAIN_SNOW already removes the weather
    # the rain beds accompany. A missing sound is a graceful no-op, so this is
    # now purely a deployment choice: copy the two folders into the install's
    # data/resources tree and they play, at the cost of SPU2 space for the rest
    # of the session.
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${PS2_APP_DIR}/data/resources/newsound/ambient"
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${PS2_APP_DIR}/data/resources/sound/loops"

    # Last, so the two removals above are already reflected in the listing.
    # Ps2ResourceManifest reads this instead of enumerating data/resources at
    # runtime; see the header of ps2_resource_manifest.cmake for why a disc
    # cannot be asked what it contains.
    COMMAND ${CMAKE_COMMAND}
            "-DROOT=${PS2_APP_DIR}/data/resources"
            "-DOUTPUT=${PS2_APP_DIR}/data/resources.manifest"
            -P "${CMAKE_SOURCE_DIR}/cmake/ps2_resource_manifest.cmake"

    COMMENT "Staging data/ into ${PS2_APP_DIR}/data"
    VERBATIM
)

# audsrv.irx used to be embedded alongside the assets; it now ships in data/irx
# and SoundManager loads it with SifExecModuleBuffer from there.
if(PS2_ENABLE_SOUND)
    set(_AUDSRV_IRX "${PS2SDK}/iop/irx/audsrv.irx")
    if(EXISTS "${_AUDSRV_IRX}")
        add_custom_command(TARGET OptiCraft POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${_AUDSRV_IRX}" "${PS2_APP_DIR}/data/irx/audsrv.irx"
            COMMENT "Packaging ${PS2_APP_DIR}/data/irx/audsrv.irx"
            VERBATIM
        )
    else()
        message(WARNING "PS2_ENABLE_SOUND is ON but audsrv.irx was not found: ${_AUDSRV_IRX}")
    endif()
endif()

# PS2 TCP multiplayer uses the modern EE-side ps2ip stack. Package the three
# IOP modules required by the Ethernet path next to the rest of the runtime
# assets so Ps2IrxLoader can bring them up lazily when Multiplayer is opened.
if(PS2_ENABLE_NETWORK)
    foreach(_PS2_NET_IRX IN ITEMS ps2dev9 netman smap)
        set(_PS2_NET_IRX_SOURCE "${PS2SDK}/iop/irx/${_PS2_NET_IRX}.irx")
        if(NOT EXISTS "${_PS2_NET_IRX_SOURCE}")
            message(FATAL_ERROR "PS2_ENABLE_NETWORK requires ${_PS2_NET_IRX_SOURCE}")
        endif()
        add_custom_command(TARGET OptiCraft POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PS2_APP_DIR}/data/irx"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${_PS2_NET_IRX_SOURCE}" "${PS2_APP_DIR}/data/irx/${_PS2_NET_IRX}.irx"
            COMMENT "Packaging ${PS2_APP_DIR}/data/irx/${_PS2_NET_IRX}.irx"
            VERBATIM
        )
    endforeach()
endif()
