# ps2_toolchain.cmake — space-tolerant wrapper around the ps2dev EE toolchain.
#
# The stock psdevwindows/ps2sdk/ps2dev.cmake bakes the $PS2SDK / $PS2DEV include
# and link paths directly into the EE_CFLAGS / EE_LDFLAGS *strings*. CMake treats
# those flag variables as verbatim text and word-splits them on spaces, so when
# the repository lives under a path containing spaces (e.g.
# "MC BETA 1.7.3 SOURCE CODE JAVA") the include paths shatter into bogus tokens
# and the compiler check fails with "linker input file not found: SOURCE" etc.
#
# Fix: before delegating to the real toolchain, rewrite the PS2DEV / PS2SDK
# environment variables to their Windows 8.3 short form (which is space-free),
# using the Scripting.FileSystemObject ShortPath. The value is passed to
# PowerShell through an env var to dodge command-line quoting entirely. The real
# ps2dev.cmake then reads the space-free paths and builds clean flag strings.
#
# On non-Windows hosts (Linux ps2dev) the SDK normally lives under a space-free
# prefix, so the conversion is simply skipped.

if(DEFINED ENV{WINDIR})
    foreach(_ps2var PS2DEV PS2SDK)
        if(DEFINED ENV{${_ps2var}} AND IS_DIRECTORY "$ENV{${_ps2var}}")
            # Pass the path via an env var so PowerShell quoting can't bite us.
            set(ENV{_PS2_SHORTEN_IN} "$ENV{${_ps2var}}")
            execute_process(
                COMMAND powershell -NoProfile -NonInteractive -Command
                        "(New-Object -ComObject Scripting.FileSystemObject).GetFolder($env:_PS2_SHORTEN_IN).ShortPath"
                OUTPUT_VARIABLE _ps2_short
                OUTPUT_STRIP_TRAILING_WHITESPACE
                RESULT_VARIABLE _ps2_rc
            )
            if(_ps2_rc EQUAL 0 AND _ps2_short)
                # Forward slashes keep the path safe inside the flag strings.
                string(REPLACE "\\" "/" _ps2_short "${_ps2_short}")
                set(ENV{${_ps2var}} "${_ps2_short}")
            endif()
        endif()
    endforeach()
    unset(ENV{_PS2_SHORTEN_IN})
endif()

# Delegate to the bundled ps2dev EE toolchain (sets the compiler, EE_CFLAGS,
# EE_LDFLAGS, PS2/EE feature vars, etc.).
include("${CMAKE_CURRENT_LIST_DIR}/../psdevwindows/ps2sdk/ps2dev.cmake")

# ps2dev.cmake injects its startup linkfile globally. Remove only that -T
# argument so cmake/ps2.cmake can select the stock script or its exception-safe
# section-GC copy at target scope without stacking two complete linker scripts.
function(_ps2_remove_stock_linkfile input_flags output_variable)
    separate_arguments(_ps2_arguments NATIVE_COMMAND "${input_flags}")
    set(_ps2_cleaned_arguments)
    foreach(_ps2_argument IN LISTS _ps2_arguments)
        string(REPLACE "\\" "/" _ps2_normalized_argument "${_ps2_argument}")
        string(TOLOWER "${_ps2_normalized_argument}" _ps2_normalized_argument)
        if(NOT _ps2_normalized_argument MATCHES
                "^-t.*/ps2sdk/ee/startup/(src/)?linkfile$")
            list(APPEND _ps2_cleaned_arguments "${_ps2_argument}")
        endif()
    endforeach()
    list(JOIN _ps2_cleaned_arguments " " _ps2_cleaned_flags)
    set(${output_variable} "${_ps2_cleaned_flags}" PARENT_SCOPE)
endfunction()

_ps2_remove_stock_linkfile("${CMAKE_EXE_LINKER_FLAGS_INIT}" _PS2_LINKER_FLAGS_CLEAN)
set(CMAKE_EXE_LINKER_FLAGS_INIT "${_PS2_LINKER_FLAGS_CLEAN}")

# Existing build directories can retain the old global -T in the cache. Clean
# that entry too so switching to this wrapper does not require a fresh tree.
if(DEFINED CMAKE_EXE_LINKER_FLAGS)
    _ps2_remove_stock_linkfile("${CMAKE_EXE_LINKER_FLAGS}" _PS2_CACHED_LINKER_FLAGS_CLEAN)
    if(NOT "${_PS2_CACHED_LINKER_FLAGS_CLEAN}" STREQUAL "${CMAKE_EXE_LINKER_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${_PS2_CACHED_LINKER_FLAGS_CLEAN}"
            CACHE STRING "Flags used by the linker during all build types" FORCE)
    endif()
endif()
