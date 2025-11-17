#pragma once
#include <stdbool.h>
#include <stddef.h>

bool cg_create(const char* name);
bool cg_move_pid(const char* name, int pid);
bool cg_move_self(const char* name);
bool cg_set_pids_max(const char* name, const char* n);         /* "max" ou número */
bool cg_set_cpu_max(const char* name, const char* max_line);   // ex: "25000 100000" ou "max 100000"
bool cg_set_memory_max(const char* name, const char* bytes);   // ex: "200M" ou "104857600"
bool cg_set_io_max(const char* name, const char* rule);        // ex: "8:0 rbps=10485760 wbps=max" (pode falhar no WSL)

bool cg_read(const char* name, const char* file, char* buf, size_t buflen); // lê arquivo do cgroup (ex.: "cpu.stat")
