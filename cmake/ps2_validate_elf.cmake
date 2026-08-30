foreach(_ps2_required_variable PS2_READELF ELF REPORT_FILE LABEL)
    if(NOT DEFINED ${_ps2_required_variable} OR "${${_ps2_required_variable}}" STREQUAL "")
        message(FATAL_ERROR "PS2 ELF validation requires ${_ps2_required_variable}")
    endif()
endforeach()

if(NOT EXISTS "${PS2_READELF}")
    message(FATAL_ERROR "PS2 readelf was not found: ${PS2_READELF}")
endif()
if(NOT EXISTS "${ELF}")
    message(FATAL_ERROR "PS2 ELF was not found: ${ELF}")
endif()

execute_process(
    COMMAND "${PS2_READELF}" -lW "${ELF}"
    RESULT_VARIABLE _ps2_program_result
    OUTPUT_VARIABLE _ps2_program_headers
    ERROR_VARIABLE _ps2_program_error
)
if(NOT _ps2_program_result EQUAL 0)
    message(FATAL_ERROR "readelf -lW failed for ${LABEL}: ${_ps2_program_error}")
endif()

string(REGEX MATCHALL "[^\r\n]*LOAD[ 	]+0x[0-9A-Fa-f]+[^\r\n]*" _ps2_load_lines "${_ps2_program_headers}")
list(LENGTH _ps2_load_lines _ps2_load_count)
if(NOT _ps2_load_count EQUAL 1)
    message(FATAL_ERROR "${LABEL}: expected exactly one LOAD segment, found ${_ps2_load_count}")
endif()

list(GET _ps2_load_lines 0 _ps2_load_line)
if(NOT _ps2_load_line MATCHES "LOAD[ 	]+0x[0-9A-Fa-f]+[ 	]+0x[0-9A-Fa-f]+[ 	]+0x[0-9A-Fa-f]+[ 	]+(0x[0-9A-Fa-f]+)[ 	]+(0x[0-9A-Fa-f]+)")
    message(FATAL_ERROR "${LABEL}: could not parse LOAD sizes: ${_ps2_load_line}")
endif()
math(EXPR _ps2_load_file_bytes "${CMAKE_MATCH_1}")
math(EXPR _ps2_load_memory_bytes "${CMAKE_MATCH_2}")

execute_process(
    COMMAND "${PS2_READELF}" -SW "${ELF}"
    RESULT_VARIABLE _ps2_section_result
    OUTPUT_VARIABLE _ps2_section_headers
    ERROR_VARIABLE _ps2_section_error
)
if(NOT _ps2_section_result EQUAL 0)
    message(FATAL_ERROR "readelf -SW failed for ${LABEL}: ${_ps2_section_error}")
endif()

set(_ps2_required_sections .text .rodata .data .bss)
set(_ps2_allocated_sections)
set(_ps2_text_bytes 0)
set(_ps2_rodata_bytes 0)
set(_ps2_data_bytes 0)
set(_ps2_bss_bytes 0)
set(_ps2_eh_frame_bytes 0)
set(_ps2_gcc_except_table_bytes 0)

string(REGEX MATCHALL "\\[[ ]*[0-9]+\\][^\r\n]*" _ps2_section_lines "${_ps2_section_headers}")
foreach(_ps2_section_line IN LISTS _ps2_section_lines)
    if(NOT _ps2_section_line MATCHES "\\[[ ]*[0-9]+\\][ 	]+([^ 	]+)[ 	]+[^ 	]+[ 	]+([0-9A-Fa-f]+)[ 	]+[0-9A-Fa-f]+[ 	]+([0-9A-Fa-f]+)[ 	]+[0-9A-Fa-f]+[ 	]+([^ 	]+)")
        continue()
    endif()

    set(_ps2_section_name "${CMAKE_MATCH_1}")
    set(_ps2_section_address_hex "${CMAKE_MATCH_2}")
    set(_ps2_section_size_hex "${CMAKE_MATCH_3}")
    set(_ps2_section_flags "${CMAKE_MATCH_4}")
    math(EXPR _ps2_section_address "0x${_ps2_section_address_hex}")
    math(EXPR _ps2_section_size "0x${_ps2_section_size_hex}")

    list(FIND _ps2_required_sections "${_ps2_section_name}" _ps2_required_index)
    if(NOT _ps2_required_index EQUAL -1)
        list(REMOVE_ITEM _ps2_required_sections "${_ps2_section_name}")
    endif()

    if(_ps2_section_name STREQUAL ".text")
        set(_ps2_text_bytes ${_ps2_section_size})
    elseif(_ps2_section_name STREQUAL ".rodata")
        set(_ps2_rodata_bytes ${_ps2_section_size})
    elseif(_ps2_section_name STREQUAL ".data")
        set(_ps2_data_bytes ${_ps2_section_size})
    elseif(_ps2_section_name STREQUAL ".bss")
        set(_ps2_bss_bytes ${_ps2_section_size})
    elseif(_ps2_section_name STREQUAL ".eh_frame")
        set(_ps2_eh_frame_bytes ${_ps2_section_size})
    elseif(_ps2_section_name STREQUAL ".gcc_except_table")
        set(_ps2_gcc_except_table_bytes ${_ps2_section_size})
    endif()

    if(_ps2_section_flags MATCHES "A" AND _ps2_section_size GREATER 0)
        list(APPEND _ps2_allocated_sections
            "${_ps2_section_address}|${_ps2_section_size}|${_ps2_section_name}")
    endif()
endforeach()

if(_ps2_required_sections)
    list(JOIN _ps2_required_sections ", " _ps2_missing_sections)
    message(FATAL_ERROR "${LABEL}: missing required ELF sections: ${_ps2_missing_sections}")
endif()

list(SORT _ps2_allocated_sections COMPARE NATURAL)
set(_ps2_previous_end 0)
set(_ps2_previous_name "")
foreach(_ps2_allocated_section IN LISTS _ps2_allocated_sections)
    string(REPLACE "|" ";" _ps2_fields "${_ps2_allocated_section}")
    list(GET _ps2_fields 0 _ps2_address)
    list(GET _ps2_fields 1 _ps2_size)
    list(GET _ps2_fields 2 _ps2_name)
    if(_ps2_address LESS _ps2_previous_end)
        message(FATAL_ERROR
            "${LABEL}: allocated section ${_ps2_name} overlaps ${_ps2_previous_name}")
    endif()
    math(EXPR _ps2_previous_end "${_ps2_address} + ${_ps2_size}")
    set(_ps2_previous_name "${_ps2_name}")
endforeach()

file(SIZE "${ELF}" _ps2_file_bytes)
string(CONCAT _ps2_report
    "label=${LABEL}\n"
    "file_bytes=${_ps2_file_bytes}\n"
    "load_file_bytes=${_ps2_load_file_bytes}\n"
    "load_memory_bytes=${_ps2_load_memory_bytes}\n"
    "text_bytes=${_ps2_text_bytes}\n"
    "rodata_bytes=${_ps2_rodata_bytes}\n"
    "data_bytes=${_ps2_data_bytes}\n"
    "bss_bytes=${_ps2_bss_bytes}\n"
    "eh_frame_bytes=${_ps2_eh_frame_bytes}\n"
    "gcc_except_table_bytes=${_ps2_gcc_except_table_bytes}\n")
file(WRITE "${REPORT_FILE}" "${_ps2_report}")
if(NOT EXISTS "${REPORT_FILE}")
    message(FATAL_ERROR "${LABEL}: failed to write ELF report: ${REPORT_FILE}")
endif()
file(READ "${REPORT_FILE}" _ps2_written_report)
if(NOT "${_ps2_written_report}" STREQUAL "${_ps2_report}")
    message(FATAL_ERROR "${LABEL}: ELF report was not written completely: ${REPORT_FILE}")
endif()
message(STATUS
    "PS2 ELF ${LABEL}: ${_ps2_file_bytes} file bytes, ${_ps2_load_file_bytes} LOAD bytes; "
    "report: ${REPORT_FILE}\n${_ps2_report}")
