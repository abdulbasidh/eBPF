#    Copyright (C) 2021 Levo Inc.
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software Foundation,
#    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

# do_or_die(COMMAND subprocess_cmd [arguments]
#           [WORKING_DIRECTORY <directory>]
#           [DESCRIPTION <command description string for logging and error messages>]
#           [DEBUG])
function(do_or_die)
    set(options "DEBUG")
    set(oneValueArgs "WORKING_DIRECTORY;DESCRIPTION")
    set(multiValueArgs "COMMAND")
    cmake_parse_arguments(PARSE_ARGV 0 FN_ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}")
    message("Executing ${FN_ARGS_DESCRIPTION}")
    if(FN_ARGS_DEBUG)
        list(JOIN FN_ARGS_COMMAND " " COMMAND_AS_STRING)
        message("COMMAND: '${COMMAND_AS_STRING}'")
        message("WORKING_DIRECTORY: '${FN_ARGS_WORKING_DIRECTORY}'")
    endif()
    execute_process(
            COMMAND ${FN_ARGS_COMMAND}
            WORKING_DIRECTORY "${FN_ARGS_WORKING_DIRECTORY}"
            RESULT_VARIABLE EXIT_CODE
            OUTPUT_VARIABLE OUT_TXT
            ERROR_VARIABLE OUT_TXT
    )
    message("${FN_ARGS_DESCRIPTION} output:\n${OUT_TXT}")
    if(NOT EXIT_CODE EQUAL 0)
        message(FATAL_ERROR "${FN_ARGS_DESCRIPTION} failed with exit code: ${EXIT_CODE}")
    endif()
endfunction()