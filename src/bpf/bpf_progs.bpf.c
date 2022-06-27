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

#include <stdio.h>
#include "bpf_progs.bpf.h"

SEC(SECNAME_HANDLE_SYS_ENTER_RECVFROM)
int PROGNAME_HANDLE_SYS_ENTER_RECVFROM(struct sys_enter_recvfrom_context *ctx){
  uint64_t tgid_pid = bpf_get_current_pid_tgid();
  if(!is_monitored_pid(tgid_pid >> 32))
    return 0;
  void * buf_ptr;
  bpf_probe_read(&buf_ptr, sizeof(buf_ptr), &ctx->dst_buf_ptr);
  return bpf_map_update_elem(&syscall_enter_exit_correlation_map, &tgid_pid, &buf_ptr, BPF_NOEXIST);
}

SEC(SECNAME_HANDLE_SYS_EXIT_RECVFROM)
int PROGNAME_HANDLE_SYS_EXIT_RECVFROM(struct sys_exit_recvfrom_context *ctx) {
  int err = 0;
  uint64_t tgid_pid = bpf_get_current_pid_tgid();
  void **buf_ptr = bpf_map_lookup_elem(&syscall_enter_exit_correlation_map, &tgid_pid);
  if(!buf_ptr)
    return 0;ssize_t recv_len;
  bpf_probe_read((void *)&recv_len, sizeof(recv_len), &ctx->ret);
  if(recv_len < 0)
    goto bail;
  size_t u_recv_len = recv_len;
  int zero = 0;
  void *scratch_map_ptr = bpf_map_lookup_elem(&data_scratch_map, &zero);
  if(!scratch_map_ptr)
    goto bail;
  if(u_recv_len > MAX_DATA_LEN)
    u_recv_len = MAX_DATA_LEN;
  //recv_len &= 0x01FFFF;
  err = bpf_probe_read(scratch_map_ptr, u_recv_len, *buf_ptr);
  if(err)
    goto bail;
  bpf_perf_event_output(ctx, &CAPTURED_DATA_MAP_NAME, BPF_F_CURRENT_CPU, scratch_map_ptr, u_recv_len);
  bail:
  return bpf_map_delete_elem(&syscall_enter_exit_correlation_map, &tgid_pid);
}

SEC(SECNAME_HANDLE_SYS_ENTER_SENDTO)
int PROGNAME_HANDLE_SYS_ENTER_SENDTO(struct sys_enter_sendto_context *ctx){
  uint64_t tgid_pid = bpf_get_current_pid_tgid();
  if(!is_monitored_pid(tgid_pid >> 32))
    return 0;
  void * buf_ptr;
  bpf_probe_read(&buf_ptr, sizeof(buf_ptr), &ctx->src_buf_ptr);
  return bpf_map_update_elem(&syscall_enter_exit_correlation_map, &tgid_pid, &buf_ptr, BPF_NOEXIST);
}

SEC(SECNAME_HANDLE_SYS_EXIT_SENDTO)
int PROGNAME_HANDLE_SYS_EXIT_SENDTO(struct sys_exit_sendto_context *ctx){
  int err = 0;
  uint64_t tgid_pid = bpf_get_current_pid_tgid();
  void **buf_ptr = bpf_map_lookup_elem(&syscall_enter_exit_correlation_map, &tgid_pid);
  if(!buf_ptr)
    return 0;ssize_t send_len;
  bpf_probe_read((void *)&send_len, sizeof(send_len), &ctx->ret);
  if(send_len < 0)
    goto bail;
  size_t u_send_len = send_len;
  int zero = 0;
  void *scratch_map_ptr = bpf_map_lookup_elem(&data_scratch_map, &zero);
  if(!scratch_map_ptr)
    goto bail;
  if(u_send_len > MAX_DATA_LEN)
    u_send_len = MAX_DATA_LEN;
  //send_len &= 0x01FFFF;
  err = bpf_probe_read(scratch_map_ptr, u_send_len, *buf_ptr);
  if(err)
    goto bail;
  bpf_perf_event_output(ctx, &CAPTURED_DATA_MAP_NAME, BPF_F_CURRENT_CPU, scratch_map_ptr, u_send_len);
  bail:
  return bpf_map_delete_elem(&syscall_enter_exit_correlation_map, &tgid_pid);
}