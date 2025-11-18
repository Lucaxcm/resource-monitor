#define _GNU_SOURCE
#include "cgroup.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

/* Raiz do cgroup v2 padrão */
static const char *CG_ROOT = "/sys/fs/cgroup";

/* Monta caminho /sys/fs/cgroup/<name>/<file> (file pode ser NULL) */
static int build_path(const char *name,
                      const char *file,
                      char *buf,
                      size_t len) {
    if (file)
        return snprintf(buf, len, "%s/%s/%s", CG_ROOT, name, file) > 0;
    else
        return snprintf(buf, len, "%s/%s", CG_ROOT, name) > 0;
}

bool cg_create(const char *name) {
    char path[256];
    if (!build_path(name, NULL, path, sizeof(path))) return false;
    if (mkdir(path, 0755) == 0) return true;
    if (errno == EEXIST) return true; /* já existe, ok */
    perror("mkdir cgroup");
    return false;
}

static bool write_file(const char *name,
                       const char *file,
                       const char *value) {
    char path[256];
    if (!build_path(name, file, path, sizeof(path))) return false;
    FILE *f = fopen(path, "w");
    if (!f) {
        perror(path);
        return false;
    }
    if (fprintf(f, "%s\n", value) < 0) {
        perror("fprintf");
        fclose(f);
        return false;
    }
    fclose(f);
    return true;
}

bool cg_move_pid(const char *name, pid_t pid) {
    char val[32];
    snprintf(val, sizeof(val), "%d", (int)pid);
    return write_file(name, "cgroup.procs", val);
}

bool cg_move_self(const char *name) {
    return cg_move_pid(name, getpid());
}

bool cg_set_cpu_max(const char *name, const char *max_line) {
    return write_file(name, "cpu.max", max_line);
}

bool cg_set_memory_max(const char *name, const char *bytes) {
    return write_file(name, "memory.max", bytes);
}

bool cg_read_file(const char *name,
                  const char *file,
                  char *buf,
                  size_t buflen) {
    char path[256];
    if (!build_path(name, file, path, sizeof(path))) return false;

    FILE *f = fopen(path, "r");
    if (!f) {
        perror(path);
        return false;
    }
    size_t n = fread(buf, 1, buflen - 1, f);
    fclose(f);
    buf[n] = '\0';
    return true;
}

bool cg_read_blkio(const char *name, char *buf, size_t buflen) {
    return cg_read_file(name, "io.stat", buf, buflen);
}

