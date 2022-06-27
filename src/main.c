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

#include <errno.h>
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bpf/bpf_prog_constants.h"

const char * help_msg =
    "Usage: levo-bpf-demo -b /path/to/bpf/object_file -p PID_to_instrument\n";

// From <bpf/libbpf.h>:
//   typedef void (*perf_buffer_sample_fn)(void *ctx, int cpu,
//                                        void *data, __u32 size);
void perf_event_callback(void *ctx, int cpu, void *data, __u32 size){
  //Print buffers forwarded to user-space by the bpf_perf_event_output() bpf helper
  //  function to the standard output
  write(STDOUT_FILENO, data, size);
}

// NOTE: We are attaching BPF programs that perform cleanup actions before we
//   attach programs that perform allocation actions.
const char *bpf_prog_names_in_attach_order[] = {
    __EXPAND_AND_QUOTE(PROGNAME_HANDLE_SYS_EXIT_RECVFROM),
    __EXPAND_AND_QUOTE(PROGNAME_HANDLE_SYS_EXIT_SENDTO),
    __EXPAND_AND_QUOTE(PROGNAME_HANDLE_SYS_ENTER_RECVFROM),
    __EXPAND_AND_QUOTE(PROGNAME_HANDLE_SYS_ENTER_SENDTO)
};

#define BPF_PROGRAM_COUNT (sizeof(bpf_prog_names_in_attach_order) / sizeof(bpf_prog_names_in_attach_order[0]))

volatile bool shutdown = false;

void int_hup_term_handler(int signum){
  fprintf(stderr, "INFO: Received signal %d.  Shutting down...\n", signum);
  shutdown = true;
}

int signals[] = { SIGINT, SIGHUP, SIGTERM};

#define SIGNALS_COUNT (sizeof(signals) / sizeof(signals[0]))

#define PERF_BUFFER_EPOLL_TIMEOUT_MS 250

int main(int arg_count, char **arg_strings){
  // Collect args and validate them
  int exit_code = 0;
  bool bad_input = false;
  int opt;
  unsigned long pid = 0;
  char *bpf_obj_path = NULL;
  while((opt = getopt(arg_count, arg_strings, "hb:p:")) != -1){
    switch(opt){
      case 'b': //BPF object file arg
        bpf_obj_path = optarg;
        break;
      case 'p': //Instrumented PID arg
        {
          unsigned long pid_len = strlen(optarg);
          char * pid_end_ptr;
          if(pid_len) {
            errno = 0;
            //See strtol(3) man page, (man 3 strotol) for more information
            pid = strtoul(optarg, &pid_end_ptr, 10);
            if(errno){
              fprintf(
                  stderr,
                  "ERROR: strtoul() failed convert PID string to unsigned long.  Error code: %d\n",
                  errno
              );
              bad_input = true;
            }
          }else{
            fprintf(stderr, "ERROR: empty string passed to -p flag.\n");
            bad_input = true;
          }
          break;
        }
      case 'h': //help arg
        printf("%s", help_msg);
        exit(EXIT_SUCCESS);
      default: //Unknown arg
        fprintf(stderr, "ERROR: Unknown argument: %c\n", opt);
        bad_input = true;
    }
  }

  if(!pid){
    fprintf(stderr, "ERROR: -p argument is required.\n");
    bad_input;
  }

  if(bpf_obj_path == NULL){
    fprintf(stderr, "ERROR: -b argument is required.\n");
    bad_input = true;
  }

  if(bad_input){
    fprintf(stderr, "\n%s", help_msg);
    exit(-EINVAL);
  }

  // Configure the libbpf library to use and enforce all of its new APIs
  // See...
  // https://github.com/libbpf/libbpf/wiki/Libbpf-1.0-migration-guide and
  // https://github.com/libbpf/libbpf/wiki/Libbpf:-the-road-to-v1.0 for more
  // information.
  libbpf_set_strict_mode(LIBBPF_STRICT_ALL);

  // Load bpf object file
  struct bpf_object *bpf_obj_ptr = bpf_object__open(bpf_obj_path);
  int err = bpf_object__load(bpf_obj_ptr);
  if(err){
    fprintf(stderr, "ERROR: bpf_object__load() failed with error code: %d\n", err);
    exit(EXIT_FAILURE);
  }

  // Add PID to filter map
  int monitored_pids_map_fd = bpf_object__find_map_fd_by_name(
      bpf_obj_ptr, __EXPAND_AND_QUOTE(MONITORED_PIDS_MAP_NAME)
  );
  char one = 1;
  err = bpf_map_update_elem(monitored_pids_map_fd, &pid, &one, BPF_NOEXIST);
  if(err){
    fprintf(stderr, "ERROR: bpf_map_update_elem() on monitored_pids map failed with error code: %d\n", err);
    exit(EXIT_FAILURE);
  }

  //Setup perf buffer
  int perf_buffer_map_fd = bpf_object__find_map_fd_by_name(
      bpf_obj_ptr, __EXPAND_AND_QUOTE(CAPTURED_DATA_MAP_NAME)
  );
  struct perf_buffer *perf_buffer_ptr = perf_buffer__new(
      perf_buffer_map_fd,
      PERF_BUFFER_PAGE_CNT,
      &perf_event_callback,
      NULL, //No callback for lost perf events
      NULL, //No ctx pointer
      NULL  //No pointer to deprecated struct perf_buffer_opts
  );

  if(!perf_buffer_ptr){
    fprintf(
        stderr,
        "ERROR: perf_buffer__new() failed to create new perf_buffer instance. Error code: %s\n",
        errno
    );
    exit(errno);
  }

  // Attach eBPF programs
  struct bpf_link *attached_programs[BPF_PROGRAM_COUNT];
  int attached_program_count = 0;
  for(int i = 0; i < BPF_PROGRAM_COUNT; ++i){
    fprintf(stderr, "Attaching eBPF program: %s\n", bpf_prog_names_in_attach_order[i]);
    struct bpf_program *prog_to_attach = bpf_object__find_program_by_name(
        bpf_obj_ptr, bpf_prog_names_in_attach_order[i]
    );
    struct bpf_link *attached_program = bpf_program__attach(prog_to_attach);
    if(!attached_program){
      fprintf(stderr, "ERROR: Failed to attach eBPF program.  Error code: %d\n", errno);
      exit_code = errno;
      goto detach_programs;
    }
    ++attached_program_count;
    attached_programs[i] = attached_program;
  }

  // Enable SIGINT/SIGHUP/SIGTERM handler
  struct sigaction sigaction_config = {
      .sa_handler = &int_hup_term_handler
  };
  for(int i = 0; i < SIGNALS_COUNT; ++i){
    err = sigaction(signals[i], &sigaction_config, NULL);
    if(err){
      fprintf(
          stderr,
          "ERROR: failed to configure custom signal handler for signal %d.  Error code: %d\n",
          signals[i],
          errno
      );
      exit_code = errno;
      goto detach_programs;
    }
  }

  // Poll the kernel for perf events from our eBPF programs
  while(!shutdown){
    perf_buffer__poll(perf_buffer_ptr, PERF_BUFFER_EPOLL_TIMEOUT_MS);
  }

  detach_programs:
  // Detach our eBPF programs in the reverse of the order in which they were attached.
  for(int i = BPF_PROGRAM_COUNT - 1; i >= 0; --i){
    if(attached_programs[i])
      bpf_link__detach(attached_programs[i]);
  }

  // Clean up perf buffer
  perf_buffer__free(perf_buffer_ptr);

  // Clean up the eBPF object
  bpf_object__close(bpf_obj_ptr);
  return exit_code;
}