#include "cgroup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * CLI mínima para o Control Group Manager.
 *
 * Comandos:
 *   cg_manager create <nome>
 *   cg_manager move   <nome> <pid>
 *   cg_manager self   <nome>
 *   cg_manager limit-cpu <nome> <linha_cpu.max>
 *   cg_manager limit-mem <nome> <bytes>
 *   cg_manager show  <nome>
 */

static void usage(const char *prog) {
    fprintf(stderr,
        "Uso:\n"
        "  %s create <nome>\n"
        "  %s move <nome> <pid>\n"
        "  %s self <nome>\n"
        "  %s limit-cpu <nome> <linha_cpu.max>\n"
        "  %s limit-mem <nome> <bytes>\n"
        "  %s show <nome>\n",
        prog, prog, prog, prog, prog, prog
    );
}

static int cmd_create(const char *name) {
    return cg_create(name) ? 0 : 1;
}

static int cmd_move(const char *name, pid_t pid) {
    return cg_move_pid(name, pid) ? 0 : 1;
}

static int cmd_self(const char *name) {
    return cg_move_self(name) ? 0 : 1;
}

static int cmd_limit_cpu(const char *name, const char *line) {
    return cg_set_cpu_max(name, line) ? 0 : 1;
}

static int cmd_limit_mem(const char *name, const char *bytes) {
    return cg_set_memory_max(name, bytes) ? 0 : 1;
}

static int cmd_show(const char *name) {
    char buf[1024];

    if (cg_read_file(name, "cpu.max", buf, sizeof(buf)))
        printf("cpu.max: %s", buf);

    if (cg_read_file(name, "cpu.stat", buf, sizeof(buf)))
        printf("cpu.stat:\n%s", buf);

    if (cg_read_file(name, "memory.max", buf, sizeof(buf)))
        printf("memory.max: %s", buf);

    if (cg_read_file(name, "memory.current", buf, sizeof(buf)))
        printf("memory.current: %s", buf);

    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argv[0]);
        return 1;
    }

    const char *cmd  = argv[1];
    const char *name = argv[2];

    if (strcmp(cmd, "create") == 0 && argc == 3)
        return cmd_create(name);
    if (strcmp(cmd, "move") == 0 && argc == 4)
        return cmd_move(name, (pid_t)atoi(argv[3]));
    if (strcmp(cmd, "self") == 0 && argc == 3)
        return cmd_self(name);
    if (strcmp(cmd, "limit-cpu") == 0 && argc == 4)
        return cmd_limit_cpu(name, argv[3]);
    if (strcmp(cmd, "limit-mem") == 0 && argc == 4)
        return cmd_limit_mem(name, argv[3]);
    if (strcmp(cmd, "show") == 0 && argc == 3)
        return cmd_show(name);

    usage(argv[0]);
    return 1;
}
