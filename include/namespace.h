#pragma once
#include <stdbool.h>

typedef struct {
  const char* name;     // "mnt","uts","ipc","pid","user","net","cgroup"
  char link[128];       // ex.: "mnt:[4026531840]"
} ns_entry_t;

/* Lê os links em /proc/<pid>/ns/<tipo> e preenche 'entries' (até maxN). Retorna quantos achou. */
int ns_list_for_pid(int pid, ns_entry_t* entries, int maxN);

/* Compara inodes de namespaces entre dois PIDs. 0 = iguais; 1 = diferentes; -1 = erro. */
int ns_compare_pids(int pidA, int pidB);

/* Agrupa processos por inode de um tipo de namespace e imprime no stdout. */
bool ns_map_processes_by_type(const char* type);

/* Mede o overhead (µs) de unshare() criando os namespaces passados em CSV. */
bool ns_measure_overhead(const char* flags_csv, int runs);
