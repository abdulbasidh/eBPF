//  Copyright (C) 2021 Levo Inc.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

#ifndef BPF_PROGS_H
#define BPF_PROGS_H

#include "bpf_prog_constants.h"

//#include <asm-generic/int-ll64.h>
#include <linux/bpf.h>
#include <linux/types.h>
#include <bpf/bpf_helpers.h>
#include <stdbool.h>
#include <stdint.h>

/*** BPF map definitions. ***/
// See https://github.com/libbpf/libbpf/wiki/Libbpf:-the-road-to-v1.0#drop-support-for-legacy-bpf-map-declaration-syntax

struct {
  __uint(type, BPF_MAP_TYPE_HASH);
  __uint(max_entries, MAX_MONITORED_PIDS);
  __type(key, uint32_t);
  __type(value, char);
} MONITORED_PIDS_MAP_NAME SEC(".maps");

struct {
  __uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
  __uint(max_entries, MAX_SUPPORTED_CPU_COUNT);
  __type(key, int);
  __type(value, uint32_t);
} CAPTURED_DATA_MAP_NAME SEC(".maps");


struct {
  __uint(type, BPF_MAP_TYPE_HASH);
  __uint(max_entries, MAX_SUPPORTED_SIMULTANEOUS_CONNECTIONS);
  __type(key, uint64_t); // See bpf_get_current_pid_gtid in the bpf-helpers(7) man page.
  __type(value, void*); // The pointer to the read or write buffer passed to sendto() or recvfrom()
} syscall_enter_exit_correlation_map SEC(".maps");

typedef struct {
  char data[MAX_DATA_LEN];
} socket_call_data;

struct{
  __uint(type, BPF_MAP_TYPE_PERCPU_ARRAY);
  __uint(max_entries, 1);
  __type(key, int);
  __type(value, char[MAX_DATA_LEN]);
} data_scratch_map SEC(".maps");

struct sys_enter_recvfrom_context{
  //common_type, common_flags, common_preempt_count, common_pid
  unsigned char unused[8];
  int32_t syscall_nr;
  /* /sys/kernel/debug/tracing/events/syscalls# cat sys_enter_recvfrom/format
   * .../sys/kernel/debug/tracing/events/syscalls/sys_enter_write# cat format
name: sys_enter_write
ID: 679
format:
	field:unsigned short common_type;	offset:0;	size:2;	signed:0;
	field:unsigned char common_flags;	offset:2;	size:1;	signed:0;
	field:unsigned char common_preempt_count;	offset:3;	size:1;	signed:0;
	field:int common_pid;	offset:4;	size:4;	signed:1;

	field:int __syscall_nr;	offset:8;	size:4;	signed:1;
	field:unsigned int fd;	offset:16;	size:8;	signed:0;
	field:const char * buf;	offset:24;	size:8;	signed:0;
	field:size_t count;	offset:32;	size:8;	signed:0;

   * field:int __syscall_nr; offset:8;       size:4; signed:1;
   * field:int fd;   offset:16;      size:8; signed:0;
   * ... */
  uint64_t fd;
  void * dst_buf_ptr;
  size_t max_recv_size;
  uint64_t flags;
  struct sockaddr * peer_sockaddr_ptr;
  uint64_t * peer_sockaddr_len_ptr;
};

struct sys_exit_recvfrom_context{
  //common_type, common_flags, common_preempt_count, common_pid
  unsigned char unused[8];
  /* /sys/kernel/debug/tracing/events/syscalls# cat sys_exit_recvfrom/format
   * ...
   * field:int __syscall_nr; offset:8;       size:4; signed:1;
   * field:long ret; offset:16;      size:8; signed:1;
   * ... */
  unsigned char pad[4];
  int64_t ret;
};

struct sys_enter_sendto_context{
  //common_type, common_flags, common_preempt_count, common_pid
  unsigned char unused[8];
  /* /sys/kernel/debug/tracing/events/syscalls# cat sys_enter_sendto/format
   * ...
   * field:int __syscall_nr; offset:8;       size:4; signed:1;
   * field:int fd;   offset:16;      size:8; signed:0;
   * ... */
  unsigned char pad[4];
  uint64_t fd;
  void * src_buf_ptr;
  size_t max_write_size;
  uint64_t flags;
  struct sockaddr * peer_sockaddr_ptr;
  uint64_t peer_sockaddr_len;
};

struct sys_exit_sendto_context{
  //common_type, common_flags, common_preempt_count, common_pid
  unsigned char unused[8];
  /* /sys/kernel/debug/tracing/events/syscalls# cat sys_exit_sendto/format
   * ...
   * field:int __syscall_nr; offset:8;       size:4; signed:1;
   * field:long ret; offset:16;      size:8; signed:1;
   * ... */
  unsigned char pad[4];
  int64_t ret;
};

#ifdef DEBUG_BPF_PRINTK
#define err(fmt, ...) { \
  const char err_msg[] = "ERROR: " fmt; \
  bpf_trace_printk((void *)&err_msg, sizeof(err_msg) __VA_OPT__(,) __VA_ARGS__);\
}
#else
#define err(fmt, ...)
#endif

static __always_inline bool is_monitored_pid(uint32_t pid){
  char *monitored_pid_map_entry = bpf_map_lookup_elem(&MONITORED_PIDS_MAP_NAME, &pid);
  if(monitored_pid_map_entry)
    return true;
  return false;
}

char LICENSE[] SEC("license") = "GPL";

#endif //BPF_PROGS_H