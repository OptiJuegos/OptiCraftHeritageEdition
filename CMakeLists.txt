cmake_minimum_required(VERSION 3.21)

project(OptiCraftHeritage
    VERSION 1.7.3
    DESCRIPTION "OptiCraft Heritage - heavily modified clean-room C++ implementation"
    LANGUAGES C CXX
)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/SourceSelection.cmake)

# StrictMathCompat reproduces the operation ordering of Java StrictMath/fdlibm.
# In particular on Broadway/PowerPC, contracting a multiply+add into FMA can
# change the last bit. Keep contraction disabled for this TU only so the rest
# of the renderer/world code keeps its normal optimization profile.
file(GLOB FDLIBM_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/java/fdlibm/*.c"
)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(
        "${CMAKE_CURRENT_SOURCE_DIR}/src/java/StrictMathCompat.cpp"
        ${FDLIBM_SOURCES}
        PROPERTIES COMPILE_OPTIONS "-ffp-contract=off"
    )
elseif(MSVC)
    set_source_files_properties(
        "${CMAKE_CURRENT_SOURCE_DIR}/src/java/StrictMathCompat.cpp"
        ${FDLIBM_SOURCES}
        PROPERTIES COMPILE_OPTIONS "/fp:strict"
    )
endif()

# PlayStation 2 build. The EE toolchain (psdevwindows/ps2sdk/ps2dev.cmake, wired
# in by the ps2-* presets) defines PS2; -DPLATFORM=PS2 works too. All PS2 logic
# lives in cmake/ps2.cmake, which builds its own target and return()s so none of
# the desktop SDL2/glad/OpenGL configuration below runs.
if(PS2 OR (DEFINED PLATFORM AND PLATFORM STREQUAL "PS2"))
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ps2.cmake)
    return()
endif()

# Nintendo Wii build. cmake/wii_toolchain.cmake (wired in by the wii-* presets)
# defines NINTENDO_WII; -DPLATFORM=WII works too. Same shape as the PS2 branch:
# cmake/wii.cmake builds its own target and return()s, so none of the desktop
# SDL2/glad/OpenGL configuration below runs.
if(NINTENDO_WII OR (DEFINED PLATFORM AND PLATFORM STREQUAL "WII"))
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/wii.cmake)
    return()
endif()

enable_testing()

function(opticraft_add_optional_test target test_source)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${test_source}")
        message(STATUS "${target} skipped: ${test_source} is not available in this source package")
        return()
    endif()

    add_executable(${target}
        "${test_source}"
        ${ARGN}
    )
    target_compile_features(${target} PRIVATE cxx_std_17)
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    add_test(NAME ${target} COMMAND ${target})
endfunction()

opticraft_add_optional_test(IntCacheTests
    tests/IntCacheTests.cpp
    src/net/minecraft/src/IntCache.cpp
)

opticraft_add_optional_test(Ps2MatrixStackTests
    tests/Ps2MatrixStackTests.cpp
    src/ps2/render/Ps2MatrixStack.cpp
)
if(TARGET Ps2MatrixStackTests)
    target_compile_definitions(Ps2MatrixStackTests PRIVATE PS2_PLATFORM=1 MC_LOG_LEVEL=0)
endif()

opticraft_add_optional_test(MathHelperFloorTests
    tests/MathHelperFloorTests.cpp
)

opticraft_add_optional_test(StructureReservationTests
    tests/StructureReservationTests.cpp
    src/net/minecraft/src/StructureBoundingBox.cpp
)

opticraft_add_optional_test(WiiContainerCursorPolicyTests
    tests/WiiContainerCursorPolicyTests.cpp
)

opticraft_add_optional_test(Ps2LightFactorCacheTests
    tests/Ps2LightFactorCacheTests.cpp
    src/ps2/render/Ps2RenderLighting.cpp
    src/ps2/render/Ps2MatrixStack.cpp
)
if(TARGET Ps2LightFactorCacheTests)
    target_compile_definitions(Ps2LightFactorCacheTests PRIVATE PS2_PLATFORM=1 MC_LOG_LEVEL=0)
endif()

opticraft_add_optional_test(WorldClientEntityIdentityTests
    tests/WorldClientEntityIdentityTests.cpp
)

opticraft_add_optional_test(JavaHashSetTests
    tests/JavaHashSetTests.cpp
    src/net/minecraft/src/ChunkCoordIntPair.cpp
)
if(TARGET JavaHashSetTests AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(JavaHashSetTests PRIVATE -std=c++17)
endif()

opticraft_add_optional_test(LocalStatsFormatTests
    tests/LocalStatsFormatTests.cpp
    src/net/minecraft/src/LocalStatsFormat.cpp
)

opticraft_add_optional_test(PcLegacyBlockCollisionMathTests
    tests/PcLegacyBlockCollisionMathTests.cpp
)

opticraft_add_optional_test(EndDragonSpawnPolicyTests
    tests/EndDragonSpawnPolicyTests.cpp
)

opticraft_add_optional_test(ChunkMemoryPolicyTests
    tests/ChunkMemoryPolicyTests.cpp
)
if(TARGET ChunkMemoryPolicyTests)
    target_compile_definitions(ChunkMemoryPolicyTests PRIVATE PS2_PLATFORM=1 MC_LOG_LEVEL=0)
endif()

opticraft_add_optional_test(Ps2MeshMemoryPolicyTests
    tests/Ps2MeshMemoryPolicyTests.cpp
)
if(TARGET Ps2MeshMemoryPolicyTests)
    target_compile_definitions(Ps2MeshMemoryPolicyTests PRIVATE PS2_PLATFORM=1 MC_LOG_LEVEL=0)
endif()

opticraft_add_optional_test(Ps2FramePacingPolicyTests
    tests/Ps2FramePacingPolicyTests.cpp
)
if(TARGET Ps2FramePacingPolicyTests)
    target_compile_definitions(Ps2FramePacingPolicyTests PRIVATE PS2_PLATFORM=1 MC_LOG_LEVEL=0)
endif()

# Keep third-party projects small: OptiCraft only needs their static libraries.
set(SDL_SHARED OFF CACHE BOOL "Build the SDL shared library" FORCE)
set(SDL_STATIC ON CACHE BOOL "Build the SDL static library" FORCE)
set(SDL_TEST OFF CACHE BOOL "Build the SDL test library" FORCE)
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "Build zlib examples" FORCE)

add_subdirectory(external/glad EXCLUDE_FROM_ALL)
add_subdirectory(external/SDL2 EXCLUDE_FROM_ALL)
add_subdirectory(external/zlib EXCLUDE_FROM_ALL)

# SDL_net links against the SDL2 static target created above, so it must be
# added afterwards. Force a static build and skip its samples; CMP0077 lets the
# normal variable override SDL_net's own option() defaults.
set(BUILD_SHARED_LIBS OFF)
set(SDL2NET_SAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/SDL_net EXCLUDE_FROM_ALL)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

# Build the desktop target from common code plus the PC tree. Foreign console
# sources are never discovered, so adding a new Wii/PS2 .cpp cannot silently
# enter a desktop link.
opticraft_collect_platform_sources(OPTICRAFT_COMMON_SOURCES pc)
set(OPTICRAFT_MINIZIP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/external/zlib/contrib/minizip/ioapi.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/zlib/contrib/minizip/unzip.c"
)
file(GLOB_RECURSE OPTICRAFT_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

if(WIN32)
    opticraft_exclude_sources(OPTICRAFT_COMMON_SOURCES "[/\\]pc[/\\]linux[/\\]")
elseif(UNIX)
    opticraft_exclude_sources(OPTICRAFT_COMMON_SOURCES "[/\\]pc[/\\]win32[/\\]")
else()
    message(FATAL_ERROR "OptiCraft currently supports Windows and Linux only")
endif()

# Each public platform facade has exactly one implementation in this target.
opticraft_select_platform_backends(OPTICRAFT_COMMON_SOURCES PC PC PC)

# Unified diagnostic verbosity, the same knob cmake/ps2.cmake and cmake/wii.cmake
# expose. The desktop target left it undefined, so platform/Log.h fell back to 0
# and every MC_LOG_* call compiled to nothing -- including the HeapValidate
# checkpoints in Minecraft.cpp, which then reported through a macro that no
# longer existed. Level 2 is needed for the MC_LOG_DEBUG lines around the
# deferred GUI/World destruction.
set(MC_LOG_LEVEL "0" CACHE STRING "Unified diagnostic verbosity: 0=off, 1=info, 2=debug, 3=trace")
set_property(CACHE MC_LOG_LEVEL PROPERTY STRINGS 0 1 2 3)

# Dedicated low-end desktop profile. The public entry point is the
# gcc32-legacy-release preset; this option is the internal architecture switch
# consumed by the platform tuning layer. Keep it separate from console-low: the
# Legacy PC build must not inherit world-generation or bounded-world policies.
option(PC_LEGACY_BUILD "Enable the 32-bit low-end PC renderer/profile" OFF)
if(PC_LEGACY_BUILD AND NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "PC_LEGACY_BUILD requires a 32-bit toolchain; use the gcc32-legacy-release preset")
endif()

add_executable(OptiCraft
    ${OPTICRAFT_COMMON_SOURCES}
    ${OPTICRAFT_MINIZIP_SOURCES}
    ${OPTICRAFT_HEADERS}
)

target_compile_features(OptiCraft PRIVATE cxx_std_17)
set_target_properties(OptiCraft PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# A level, not a boolean, so it is passed through as-is rather than through
# $<BOOL:>, which would collapse 2 to 1.
target_compile_definitions(OptiCraft PRIVATE MC_LOG_LEVEL=${MC_LOG_LEVEL})
if(PC_LEGACY_BUILD)
    target_compile_definitions(OptiCraft PRIVATE PC_LEGACY_BUILD=1)
endif()

# Link-Time Optimization for Release. The port is built from hundreds of tiny
# cross-translation-unit accessors (getBlockId, getSavedLightValue, Chunk /
# NibbleArray getters) called millions of times per frame in the hot loops;
# without LTO the compiler cannot inline them across .cpp boundaries.
#
# OFF by default: MinGW's bundled ld can fail the final link with
# "plugin needed to handle lto object" because it isn't invoked with GCC's LTO
# linker plugin. The bulk of the codegen win already comes from -O2/-O3 (per-TU
# inlining); LTO only adds the cross-TU part. Turn this on (and make sure the
# toolchain uses gcc-ar / gcc-ranlib and -fuse-linker-plugin) to chase the rest.
option(OPTICRAFT_ENABLE_LTO "Enable Link-Time Optimization for Release builds" OFF)
if(OPTICRAFT_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error LANGUAGES CXX)
    if(ipo_supported)
        set_target_properties(OptiCraft PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    else()
        message(WARNING "IPO/LTO requested but not supported by this toolchain, skipping: ${ipo_error}")
    endif()
endif()

target_include_directories(OptiCraft PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pc"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/stb"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/miniaudio"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/zlib/contrib/minizip"
)

target_link_libraries(OptiCraft PRIVATE
    glad
    SDL2::SDL2-static
    SDL2_net::SDL2_net-static
    zlibstatic
    OpenGL::GL
    Threads::Threads
)

if(WIN32)
    target_compile_definitions(OptiCraft PRIVATE
        MC_WIN32
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
    target_link_libraries(OptiCraft PRIVATE shell32 psapi d3d9)

    if(MSVC)
        target_compile_options(OptiCraft PRIVATE /MP /bigobj /utf-8)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(OptiCraft PRIVATE
            -Wa,-mbig-obj
            $<$<CONFIG:Release>:-Os>
            $<$<NOT:$<CONFIG:Release>>:-g>
        )
        if(PC_LEGACY_BUILD)
            # Tune scheduling for Bonnell without raising the inherited SSE2 ISA baseline.
            target_compile_options(OptiCraft PRIVATE
                $<$<CONFIG:Release>:-O2>
                -mtune=bonnell
                $<$<CONFIG:Release>:-fomit-frame-pointer>
                $<$<NOT:$<CONFIG:Release>>:-fno-omit-frame-pointer>
                -fno-math-errno
                -fno-trapping-math
            )
        else()
            target_compile_options(OptiCraft PRIVATE
                -fno-omit-frame-pointer
            )
        endif()
        target_link_options(OptiCraft PRIVATE
            -static
            -fno-ident
            -fno-asynchronous-unwind-tables
            -fvisibility=hidden
            $<$<CONFIG:Release>:-s>
        )
    endif()
elseif(UNIX)
    target_compile_definitions(OptiCraft PRIVATE MC_LINUX)
endif()

# Put every configuration in a predictable folder instead of beside CMake's
# generated files. Explicit per-config properties avoid an extra Debug/Debug.
foreach(config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${config}" config_upper)
    set_target_properties(OptiCraft PROPERTIES
        "RUNTIME_OUTPUT_DIRECTORY_${config_upper}"
        "${CMAKE_CURRENT_SOURCE_DIR}/bin/${config}"
    )
endforeach()

if(PC_LEGACY_BUILD)
    set_target_properties(OptiCraft PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_RELEASE
        "${CMAKE_CURRENT_SOURCE_DIR}/bin/gcc32-legacy-release"
    )
endif()

if(CMAKE_CONFIGURATION_TYPES)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT OptiCraft)
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Source Files"
    FILES ${OPTICRAFT_COMMON_SOURCES} ${OPTICRAFT_HEADERS}
)
source_group("Third Party\\minizip" FILES ${OPTICRAFT_MINIZIP_SOURCES})
