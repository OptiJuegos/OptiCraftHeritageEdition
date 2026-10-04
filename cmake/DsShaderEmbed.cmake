# DsShaderEmbed.cmake -- embed a picasso SHBIN binary into a C header.
#
# picasso's -h switch only emits the shader's uniform-register defines
# (VSH_FVEC_*/VSH_ULEN_*); the assembled binary itself still has to reach the
# C++ side somehow. The devkitPro makefiles bridge that with bin2s piped into
# the assembler; CMake has no equivalent helper, and add_custom_command has no
# portable output redirection, so this script reads the .bin back as hex and
# writes a header with the same word array bin2s would have assembled.
#
# The 3DS ABI is little-endian, so the on-disk byte order is b0 b1 b2 b3 for
# each u32: every 8-digit hex group has to be byte-reversed to recover the
# value the CPU will see at runtime.
#
# Usage:
#   cmake -DDsShaderEmbed_INPUT=<shbin> -DDsShaderEmbed_OUTPUT=<header>
#         -DDsShaderEmbed_SYMBOL=<identifier> -P cmake/DsShaderEmbed.cmake

if(NOT DEFINED DsShaderEmbed_INPUT OR NOT DEFINED DsShaderEmbed_OUTPUT
   OR NOT DEFINED DsShaderEmbed_SYMBOL)
    message(FATAL_ERROR
        "DsShaderEmbed.cmake: DsShaderEmbed_INPUT, DsShaderEmbed_OUTPUT and "
        "DsShaderEmbed_SYMBOL must all be defined")
endif()

if(NOT EXISTS "${DsShaderEmbed_INPUT}")
    message(FATAL_ERROR "DsShaderEmbed.cmake: input '${DsShaderEmbed_INPUT}' does not exist")
endif()

file(READ "${DsShaderEmbed_INPUT}" _hex HEX)
string(LENGTH "${_hex}" _len)
math(EXPR _words "${_len} / 8")
math(EXPR _len_check "${_words} * 8")
if(NOT _len EQUAL _len_check)
    message(FATAL_ERROR "DsShaderEmbed.cmake: '${DsShaderEmbed_INPUT}' is not a whole number of u32 words")
endif()

set(_body "")
set(_i 0)
while(_i LESS _words)
    math(EXPR _off "${_i} * 8")
    string(SUBSTRING "${_hex}" "${_off}" 8 _word)
    string(SUBSTRING "${_word}" 6 2 _b3)
    string(SUBSTRING "${_word}" 4 2 _b2)
    string(SUBSTRING "${_word}" 2 2 _b1)
    string(SUBSTRING "${_word}" 0 2 _b0)
    string(APPEND _body "    0x${_b3}${_b2}${_b1}${_b0},\n")
    math(EXPR _i "${_i} + 1")
endwhile()

file(WRITE "${DsShaderEmbed_OUTPUT}"
"// Generated from ${DsShaderEmbed_INPUT} by cmake/DsShaderEmbed.cmake.
// Do not edit: rebuild regenerates it from src/3ds/render/DsShader.v.pica.
#pragma once
#include <cstdint>
const std::uint32_t ${DsShaderEmbed_SYMBOL}[] = {
${_body}};
const std::uint32_t ${DsShaderEmbed_SYMBOL}_size = sizeof(${DsShaderEmbed_SYMBOL}) / 4;
")
