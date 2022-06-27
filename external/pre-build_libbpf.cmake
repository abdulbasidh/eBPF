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

message("Pre-building libbpf.")

string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWERCASE)
if(BUILD_TYPE_LOWERCASE STREQUAL debug)
    set(CFLAGS_ARG "'CFLAGS=-O0 -g -Werror -Wall'")
else()
    # See if we can skip the debug info and bump optimization for "release" builds...
    set(CFLAGS_ARG "'CFLAGS=-O3 -Werror -Wall'")
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/libbpf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

execute_process(
        COMMAND make -j${CORE_COUNT} BUILD_STATIC_ONLY=y ${CFLAGS_ARG} PREFIX=${STAGING_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libbpf/src
        RESULT_VARIABLE LIBBPF_BUILD_EXIT_CODE
        OUTPUT_VARIABLE LIBBPF_MAKE_OUT
        ERROR_VARIABLE LIBBPF_MAKE_OUT
)
message("libbpf 'make' output: ${LIBBPF_MAKE_OUT}")
if(NOT LIBBPF_BUILD_EXIT_CODE EQUAL 0)
    message(FATAL_ERROR "Failed to pre-build libbpf")
endif()

execute_process(
        COMMAND make -j${CORE_COUNT} install BUILD_STATIC_ONLY=y ${CFLAGS_ARG} PREFIX=${STAGING_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libbpf/src
        RESULT_VARIABLE LIBBPF_INSTALL_EXIT_CODE
        OUTPUT_VARIABLE LIBBPF_MAKE_OUT
        ERROR_VARIABLE LIBBPF_MAKE_OUT
)
message("libbpf 'make install' output: ${LIBBPF_MAKE_OUT}")
if(NOT LIBBPF_INSTALL_EXIT_CODE EQUAL 0)
    message(FATAL_ERROR "Failed to install pre-built libbpf")
endif()
