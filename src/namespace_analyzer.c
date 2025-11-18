#define _GNU_SOURCE
#include "namespace.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Lê o número dentro de "algo:[12345]" */
static int parse_ns_id(const char *link, unsigned long *id) {
    const char *p = strchr(link, '[');
    if (!p) return 0;
    char *end = NULL;
    unsigned long v = strtoul(p + 1, &end, 10);
    if (!end || *end != ']') return 0;
    *id = v;
    return 1;
}

/* Auxiliar para montar o caminho /proc/<pid>/ns/<name> */
static int build_ns_path(pid_t pid, const char *name, char *buf, size_t len) {
    return snprintf(buf, len, "/proc/%d/ns/%s", (int)pid, name) > 0;
}

/* Implementação da API declarada no header */

bool ns_read_ids(pid_t pid, ns_ids_t *out) {
    if (!out) return false;

    const char *names[] = {"mnt","pid","uts","ipc","net","user","cgroup"};
    unsigned long *slots[] = {
        &out->mnt,&out->pid,&out->uts,&out->ipc,
        &out->net,&out->user,&out->cgroup
    };

    for (size_t i = 0; i < sizeof(names)/sizeof(names[0]); ++i) {
        char path[64], link[128];
        if (!build_ns_path(pid, names[i], path, sizeof(path)))
            return false;

        ssize_t n = readlink(path, link, sizeof(link) - 1);
        if (n < 0) return false;
        link[n] = '\0';

        if (!parse_ns_id(link, slots[i])) return false;
    }
    return true;
}

bool ns_equal(const ns_ids_t *a, const ns_ids_t *b) {
    return a->mnt == b->mnt   &&
           a->pid == b->pid   &&
           a->uts == b->uts   &&
           a->ipc == b->ipc   &&
           a->net == b->net   &&
           a->user == b->user &&
           a->cgroup == b->cgroup;
}

/* ------------ CLI simples para o namespace analyzer ------------ */

static void usage(const char *prog) {
    fprintf(stderr,
        "Uso:\n"
        "  %s list <pid>             - lista namespaces de um processo\n"
        "  %s compare <pid1> <pid2>  - compara namespaces de dois processos\n"
        "  %s report                 - relatório simples de namespaces do sistema\n",
        prog, prog, prog
    );
}

/* Imprime os IDs de um processo */
static int cmd_list(pid_t pid) {
    ns_ids_t ns;
    if (!ns_read_ids(pid, &ns)) {
        perror("ns_read_ids");
        return 1;
    }
    printf("PID %d:\n", (int)pid);
    printf("  mnt    : %lu\n", ns.mnt);
    printf("  pid    : %lu\n", ns.pid);
    printf("  uts    : %lu\n", ns.uts);
    printf("  ipc    : %lu\n", ns.ipc);
    printf("  net    : %lu\n", ns.net);
    printf("  user   : %lu\n", ns.user);
    printf("  cgroup : %lu\n", ns.cgroup);
    return 0;
}

/* Compara dois PIDs */
static int cmd_compare(pid_t a, pid_t b) {
    ns_ids_t na, nb;
    if (!ns_read_ids(a, &na) || !ns_read_ids(b, &nb)) {
        perror("ns_read_ids");
        return 1;
    }
    printf("Comparando %d e %d:\n", (int)a, (int)b);
    printf("  PID ns iguais? %s\n", na.pid == nb.pid ? "SIM" : "NAO");
    printf("  NET ns iguais? %s\n", na.net == nb.net ? "SIM" : "NAO");
    printf("  MNT ns iguais? %s\n", na.mnt == nb.mnt ? "SIM" : "NAO");
    printf("  USER ns iguais? %s\n", na.user == nb.user ? "SIM" : "NAO");
    return 0;
}

/* Relatório simples: conta quantos processos por ID de namespace de PID */
static int cmd_report(void) {
    DIR *d = opendir("/proc");
    if (!d) {
        perror("opendir /proc");
        return 1;
    }

    /* Contamos até um máximo simples para não complicar demais */
    struct {
        unsigned long id;
        int count;
    } entries[1024];
    int used = 0;

    struct dirent *de;
    while ((de = readdir(d))) {
        char *end = NULL;
        long pid = strtol(de->d_name, &end, 10);
        if (*end != '\0') continue; /* não é número */

        ns_ids_t ns;
        if (!ns_read_ids((pid_t)pid, &ns)) continue;

        /* procura id já existente */
        int found = -1;
        for (int i = 0; i < used; ++i) {
            if (entries[i].id == ns.pid) {
                found = i;
                break;
            }
        }
        if (found >= 0) {
            entries[found].count++;
        } else if (used < (int)(sizeof(entries)/sizeof(entries[0]))) {
            entries[used].id = ns.pid;
            entries[used].count = 1;
            used++;
        }
    }
    closedir(d);

    printf("Relatorio de namespaces PID (/proc/<pid>/ns/pid):\n");
    for (int i = 0; i < used; ++i) {
        printf("  ns_pid=%lu : %d processos\n",
               entries[i].id, entries[i].count);
    }
    return 0;
}

/* Lista todos os processos pertencentes a um namespace específico */
static int cmd_find(const char *ns_type, unsigned long ns_id) {
    DIR *d = opendir("/proc");
    if (!d) {
        perror("opendir /proc");
        return 1;
    }

    struct dirent *de;
    while ((de = readdir(d))) {
        char *end = NULL;
        long pid = strtol(de->d_name, &end, 10);
        if (*end != '\0') continue;

        ns_ids_t ns;
        if (!ns_read_ids((pid_t)pid, &ns)) continue;

        unsigned long current = 0;

        if      (strcmp(ns_type,"mnt")==0)    current = ns.mnt;
        else if (strcmp(ns_type,"pid")==0)    current = ns.pid;
        else if (strcmp(ns_type,"uts")==0)    current = ns.uts;
        else if (strcmp(ns_type,"ipc")==0)    current = ns.ipc;
        else if (strcmp(ns_type,"net")==0)    current = ns.net;
        else if (strcmp(ns_type,"user")==0)   current = ns.user;
        else if (strcmp(ns_type,"cgroup")==0) current = ns.cgroup;
        else {
            fprintf(stderr,"Namespace invalido: %s\n", ns_type);
            closedir(d);
            return 1;
        }

        if (current == ns_id)
            printf("%ld\n", pid);
    }

    closedir(d);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {

        if (strcmp(argv[1], "find") == 0 && argc == 4) {
            return cmd_find(argv[2], strtoul(argv[3], NULL, 10));
        }

        usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "list") == 0 && argc == 3) {
        return cmd_list((pid_t)atoi(argv[2]));
    } else if (strcmp(argv[1], "compare") == 0 && argc == 4) {
        return cmd_compare((pid_t)atoi(argv[2]), (pid_t)atoi(argv[3]));
    } else if (strcmp(argv[1], "report") == 0 && argc == 2) {
        return cmd_report();
    }

    usage(argv[0]);
    return 1;
}
