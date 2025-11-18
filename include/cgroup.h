#ifndef CGROUP_H
#define CGROUP_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

/*
 * Funções mínimas para trabalhar com cgroup v2 em /sys/fs/cgroup.
 * Cada função basicamente monta um caminho, abre o arquivo e escreve
 * ou lê o valor.
 */

/* Cria um cgroup filho de /sys/fs/cgroup/<name> */
bool cg_create(const char *name);

/* Move um PID para o cgroup (escreve em cgroup.procs) */
bool cg_move_pid(const char *name, pid_t pid);

/* Move o próprio processo (getpid()) */
bool cg_move_self(const char *name);

/*
 * Limites básicos:
 *  - CPU: escreve a linha em cpu.max   (ex.: "25000 100000" ou "max 100000")
 *  - Memória: escreve em memory.max   (ex.: "104857600" para 100MB)
 */
bool cg_set_cpu_max(const char *name, const char *max_line);
bool cg_set_memory_max(const char *name, const char *bytes);

/* Lê um arquivo arbitrário do cgroup, ex.: "cpu.stat" ou "memory.current" */
bool cg_read_file(const char *name,
                  const char *file,
                  char *buf,
                  size_t buflen);

#endif /* CGROUP_H */
