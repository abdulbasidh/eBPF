//  Copyright (C) 2021 Levo Inc.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

#ifndef BPF_PROG_CONSTANTS_H
#define BPF_PROG_CONSTANTS_H

#define MAX_MONITORED_PIDS 1

//128kiB
#define PERF_BUFFER_SIZE_BYTES (2 << 17)
//TODO: Conditional definitions based on architecture to support aarch64
#define PAGE_SIZE_BYTES 4096
#define PERF_BUFFER_PAGE_CNT PERF_BUFFER_SIZE_BYTES / PAGE_SIZE_BYTES

#define MAX_SUPPORTED_CPU_COUNT 512

#define MAX_SUPPORTED_SIMULTANEOUS_CONNECTIONS 42

#define MAX_DATA_LEN 32768

// See https://gcc.gnu.org/onlinedocs/gcc-4.1.2/cpp/Stringification.html
//   for more information
#define __QUOTE(L)  #L
#define __EXPAND_AND_QUOTE(M) __QUOTE(M)

#define MONITORED_PIDS_MAP_NAME monitored_pids
#define CAPTURED_DATA_MAP_NAME captured_data

#define SECNAME_HANDLE_SYS_ENTER_RECVFROM "tracepoint/syscalls/sys_enter_recvfrom"
#define SECNAME_HANDLE_SYS_EXIT_RECVFROM "tracepoint/syscalls/sys_exit_recvfrom"
#define SECNAME_HANDLE_SYS_ENTER_SENDTO "tracepoint/syscalls/sys_enter_sendto"
#define SECNAME_HANDLE_SYS_EXIT_SENDTO "tracepoint/syscalls/sys_exit_sendto"

#define PROGNAME_HANDLE_SYS_ENTER_RECVFROM handle_enter_recvfrom
#define PROGNAME_HANDLE_SYS_EXIT_RECVFROM handle_exit_recvfrom
#define PROGNAME_HANDLE_SYS_ENTER_SENDTO handle_enter_sendto
#define PROGNAME_HANDLE_SYS_EXIT_SENDTO handle_exit_sendto

#endif//BPF_PROG_CONSTANTS_H