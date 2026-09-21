# Shared source-selection helpers for OptiCraft targets.
#
# Source discovery is split into common code and one platform tree. Backends
# that export the same public symbols are selected explicitly, so foreign
# platform implementations never enter a target by accident.

function(mcbeta_collect_common_sources out_var)
    file(GLOB_RECURSE _sources CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/src/client/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/client/*.c"
        "${CMAKE_SOURCE_DIR}/src/external/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/external/*.c"
        "${CMAKE_SOURCE_DIR}/src/java/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/java/*.c"
        "${CMAKE_SOURCE_DIR}/src/mods/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/mods/*.c"
        "${CMAKE_SOURCE_DIR}/src/net/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/net/*.c"
        "${CMAKE_SOURCE_DIR}/src/platform/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/platform/*.c"
        "${CMAKE_SOURCE_DIR}/src/util/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/util/*.c"
    )
    set(${out_var} "${_sources}" PARENT_SCOPE)
endfunction()

function(mcbeta_collect_platform_sources out_var platform_dir)
    mcbeta_collect_common_sources(_sources)
    file(GLOB_RECURSE _platform_sources CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/src/${platform_dir}/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/${platform_dir}/*.c"
    )
    list(APPEND _sources ${_platform_sources})
    set(${out_var} "${_sources}" PARENT_SCOPE)
endfunction()

function(mcbeta_exclude_sources list_var)
    set(_sources "${${list_var}}")
    foreach(_regex IN LISTS ARGN)
        list(FILTER _sources EXCLUDE REGEX "${_regex}")
    endforeach()
    set(${list_var} "${_sources}" PARENT_SCOPE)
endfunction()

# Console-local stats use LocalStatsFormat + PlatformStorage and never instantiate
# the desktop synchronizer. Keep its thread/JSON/MD5 implementation out of
# console targets entirely instead of compiling an unreachable subsystem.
function(mcbeta_exclude_remote_stats_sources list_var)
    set(_sources "${${list_var}}")
    list(FILTER _sources EXCLUDE REGEX
        "[/\\](StatsSyncher|ThreadStatSyncherReceive|ThreadStatSyncherSend|MD5String|J_.*)\\.cpp$")
    set(${list_var} "${_sources}" PARENT_SCOPE)
endfunction()

function(mcbeta_select_backend list_var relative_dir family backend source_pattern)
    set(_sources "${${list_var}}")
    list(FILTER _sources EXCLUDE REGEX "${source_pattern}")

    set(_backend_source "${CMAKE_SOURCE_DIR}/src/${relative_dir}/${family}_${backend}.cpp")
    if(NOT EXISTS "${_backend_source}")
        message(FATAL_ERROR "${family} backend source not found: ${_backend_source}")
    endif()

    list(APPEND _sources "${_backend_source}")
    set(${list_var} "${_sources}" PARENT_SCOPE)
    message(STATUS "${family} backend: ${backend}")
endfunction()

function(mcbeta_select_platform_backends list_var platform render_backend sound_backend)
    mcbeta_select_backend(${list_var} platform RenderAPI ${render_backend}
        "[/\\\\]platform[/\\\\]RenderAPI_(PC|GL|GX_WII|GS_PS2)\\.cpp$")
    mcbeta_select_backend(${list_var} platform RenderTerrainAPI ${render_backend}
        "[/\\\\]platform[/\\\\]RenderTerrainAPI_(PC|GL|GX_WII|GS_PS2)\\.cpp$")
    mcbeta_select_backend(${list_var} platform/audio SoundManager ${sound_backend}
        "[/\\\\]platform[/\\\\]audio[/\\\\]SoundManager_(PC|WII|PS2)\\.cpp$")
    mcbeta_select_backend(${list_var} platform/storage StorageBackend ${platform}
        "[/\\\\]platform[/\\\\]storage[/\\\\]StorageBackend_(PC|WII|PS2)\\.cpp$")
    mcbeta_select_backend(${list_var} platform InputBackend ${platform}
        "[/\\\\]platform[/\\\\]InputBackend_(PC|WII|PS2)\\.cpp$")
    mcbeta_select_backend(${list_var} platform LegacyControlPromptBackend ${platform}
        "[/\\\\]platform[/\\\\]LegacyControlPromptBackend_(PC|WII|PS2)\\.cpp$")
    mcbeta_select_backend(${list_var} platform Resources ${platform}
        "[/\\\\]platform[/\\\\]Resources_(PC|WII|PS2)\\.cpp$")
    mcbeta_select_backend(${list_var} platform Diagnostics ${platform}
        "[/\\\\]platform[/\\\\]Diagnostics_(PC|WII|PS2)\\.cpp$")
    mcbeta_select_backend(${list_var} platform GameSettingsBackend ${platform}
        "[/\\\\]platform[/\\\\]GameSettingsBackend_(PC|WII|PS2)\\.cpp$")
    mcbeta_select_backend(${list_var} platform PlatformUserSettings ${platform}
        "[/\\\\]platform[/\\\\]PlatformUserSettings_(PC|WII|PS2)\\.cpp$")
    mcbeta_select_backend(${list_var} platform Profiler ${platform}
        "[/\\\\]platform[/\\\\]Profiler_(PC|WII|PS2)\\.cpp$")
    mcbeta_select_backend(${list_var} platform ClientProfilerBackend ${platform}
        "[/\\\\]platform[/\\\\]ClientProfilerBackend_(PC|WII|PS2)\\.cpp$")
    mcbeta_select_backend(${list_var} platform ClientPlatformPolicy ${platform}
        "[/\\\\]platform[/\\\\]ClientPlatformPolicy_(PC|WII|PS2)\\.cpp$")
    mcbeta_select_backend(${list_var} platform ScreenshotBackend ${platform}
        "[/\\\\]platform[/\\\\]ScreenshotBackend_(PC|WII|PS2)\\.cpp$")
    mcbeta_select_backend(${list_var} platform RenderLightingProfile ${platform}
        "[/\\\\]platform[/\\\\]RenderLightingProfile_(PC|WII|PS2)\\.cpp$")
    mcbeta_select_backend(${list_var} platform TextureResidencyPolicy ${platform}
        "[/\\\\]platform[/\\\\]TextureResidencyPolicy_(PC|WII|PS2)\\.cpp$")
    set(${list_var} "${${list_var}}" PARENT_SCOPE)
endfunction()
