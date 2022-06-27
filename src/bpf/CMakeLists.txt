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

set(CMAKE_C_COMPILER clang)

# Notes on C Flags
# -Werror: treat warnings as errors.  Especially helpful when Clang can't figure out how to unroll a loop.
# -O2: Optimize generated code
# -target bpf: output BPF bytecode
# -save-temps: Preserve the intermediate BPF assembly file generated during the compilation process
set(CMAKE_C_FLAGS
        "-Werror -O2 -target bpf -g -save-temps"
)

add_library(compile_bpf OBJECT bpf_progs.bpf.c)
add_custom_target(
        # create a virtual target add it to the default make target
        bpf_obj ALL
        # depend on the output of the custom command below
        DEPENDS bpf_obj_file_copy
)

if(NOT DEFINED BPF_OBJ_FILE)
    set(BPF_OBJ_FILE bpf_progs.bpf.o)
endif()

add_custom_command(
        OUTPUT bpf_obj_file_copy
        COMMAND cp ARGS $<TARGET_OBJECTS:compile_bpf> ${CMAKE_CURRENT_BINARY_DIR}/${BPF_OBJ_FILE}
        DEPENDS compile_bpf $<TARGET_OBJECTS:compile_bpf>
)