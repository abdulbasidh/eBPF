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

include(levo_helpers.cmake)

cmake_minimum_required(VERSION 3.18)
project(
        ebpf_demo
        VERSION 1.0.0
        DESCRIPTION "Levo.ai's eBPF demonstration application"
        LANGUAGES C
)
set(CMAKE_C_STANDARD 99)



if(CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR
            "This project enforces out-of-source builds.  Please create a new, empty "
            "directory, change to that directory, and run...\n"
            "cmake ${CMAKE_SOURCE_DIR}\n"
            "...to generate makefiles."
            )
endif()

# Figure out whether we are (probably) in a git clone, or building from an
# unpacked tarball or zipfile...
execute_process(
        COMMAND git status
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE GIT_STATUS_EXIT_CODE
)

if(GIT_STATUS_EXIT_CODE EQUAL 0)
    if(NOT ${CACHE}SUBMODULES_INITIALIZED)
        do_or_die(
                COMMAND git submodule update --init --force --recursive
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
        set(SUBMODULES_INITIALIZED TRUE CACHE BOOL "Project git submodules already initialized" FORCE)
    endif()
else()
    set(LIBBPF_GIT_TAG v0.6.1)
    do_or_die(
            COMMAND git clone https://github.com/libbpf/libbpf.git
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external"
            DESCRIPTION "Cloning libbpf to '${CMAKE_SOURCE_DIR}/external'"
    )
    do_or_die(
            COMMAND git checkout ${LIBBPF_GIT_TAG}
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/libbpf"
            DESCRIPTION "Checking out libbpf git tag: ${LIBBPF_GIT_TAG}"
    )
endif()

cmake_host_system_information(RESULT CORE_COUNT QUERY NUMBER_OF_PHYSICAL_CORES)

# STAGING_DIR is a scratch directory where we can "make install" dependencies
# that we have to pre-build and install for them to work correctly.
set(STAGING_DIR ${CMAKE_BINARY_DIR}/staging)
set(CMAKE_PREFIX_PATH ${STAGING_DIR})
# If there are duplicate headers on the system, prefer those in "${STAGING_DIR}/include"
include_directories(BEFORE SYSTEM "${STAGING_DIR}/include")
link_directories(BEFORE "${STAGING_DIR}/lib64")

add_subdirectory(external)
add_subdirectory(src)
