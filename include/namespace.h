#ifndef NAMESPACE_H
#define NAMESPACE_H

#include <stdbool.h>
#include <sys/types.h>

/*
 * Utilitários simples para trabalhar com namespaces.
 * A estratégia é ler os links simbólicos em caminhos como:
 *   /proc/<pid>/ns/net
 *   /proc/<pid>/ns/pid
 *   /proc/<pid>/ns/mnt
 * e extrair o número dentro dos colchetes, ex.: "net:[4026531993]".
 */

/* Tipos de namespace que vamos suportar */
typedef struct {
    unsigned long mnt;
    unsigned long pid;
    unsigned long uts;
    unsigned long ipc;
    unsigned long net;
    unsigned long user;
    unsigned long cgroup;
} ns_ids_t;

/* Lê todos os namespaces de um processo */
bool ns_read_ids(pid_t pid, ns_ids_t *out);

/* Compara dois conjuntos de IDs (true se forem iguais) */
bool ns_equal(const ns_ids_t *a, const ns_ids_t *b);

#endif /* NAMESPACE_H */
