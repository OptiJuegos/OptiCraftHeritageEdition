# wii_check_no_tls.cmake — post-link assertion that the Wii ELF contains no
# thread-local storage.
#
# Invoked as a POST_BUILD step from cmake/wii.cmake:
#     cmake -DREADELF=<powerpc-eabi-readelf> -DELF=<file> -P wii_check_no_tls.cmake
#
# Why this exists
# ---------------
# devkitPPC compiles `thread_local` with the local-exec model (r2+TPREL), but
# libogc's linker script defines no .tdata/.tbss output section. The link
# therefore produces no PT_TLS segment, every .tbss input section is orphan-
# placed at TLS offset 0 -- so *all* thread_locals in the program alias one word
# -- and r2 holds _SDA2_BASE_, not a thread pointer. The variables end up reading
# and writing live .eh_frame_hdr data.
#
# Nothing warns about any of that: it compiles, it links, and it corrupts. It
# cost several debugging sessions the first time, via stb_image's
# "flip vertically on load" flag reading as true and loading every texture upside
# down (see src/external/stb_image.cpp). Assert on it instead of rediscovering it.
#
# The check is advisory when readelf is unavailable and fatal when TLS is found.

if(NOT EXISTS "${ELF}")
    message(FATAL_ERROR "wii_check_no_tls: no such file: ${ELF}")
endif()

execute_process(
    COMMAND "${READELF}" -SW "${ELF}"
    OUTPUT_VARIABLE _sections
    RESULT_VARIABLE _readelf_rc
    ERROR_QUIET
)

if(NOT _readelf_rc EQUAL 0)
    message(WARNING
        "wii_check_no_tls: '${READELF}' failed (${_readelf_rc}); skipping the TLS check. "
        "thread_local is unusable on this toolchain -- verify by hand if you added any.")
    return()
endif()

# Section names may be elided by readelf (".tbss._ZL8fl[...]"), so match the
# prefix rather than a whole name.
string(REGEX MATCHALL "\\.t(data|bss)[^ \t\n]*" _tls_sections "${_sections}")

if(_tls_sections)
    list(REMOVE_DUPLICATES _tls_sections)
    string(REPLACE ";" "\n      " _tls_pretty "${_tls_sections}")
    message(FATAL_ERROR
        "Thread-local storage linked into ${ELF}:\n"
        "      ${_tls_pretty}\n\n"
        "devkitPPC has no working TLS: there is no PT_TLS segment, all of these "
        "alias the same word, and r2 is _SDA2_BASE_ rather than a thread pointer, "
        "so they read and write live .eh_frame_hdr data.\n"
        "Fix the owning code: use a vendored library's opt-out (stb_image has "
        "STBI_NO_THREAD_LOCALS), or make the variable a plain static.")
endif()
