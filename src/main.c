#define _GNU_SOURCE
#include "monitor.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

/* tempo em ms desde epoch */
static long now_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long)(tv.tv_sec * 1000L + tv.tv_usec / 1000L);
}

static void usage(const char *prog) {
    fprintf(stderr,
        "Uso: %s <pid> <intervalo_ms> <samples>\n"
        "Exemplo: %s 1234 500 20\n",
        prog, prog
    );
}

int main(int argc, char **argv) {
    if (argc != 4) {
        usage(argv[0]);
        return 1;
    }

    pid_t pid       = (pid_t)atoi(argv[1]);
    int interval_ms = atoi(argv[2]);
    int samples     = atoi(argv[3]);

    if (pid <= 0 || interval_ms <= 0 || samples <= 0) {
        fprintf(stderr, "Parametros invalidos.\n");
        return 1;
    }

    char proc_path[64];
    snprintf(proc_path, sizeof(proc_path), "/proc/%d", (int)pid);
    if (access(proc_path, R_OK) != 0) {
        fprintf(stderr, "Nao consigo ler %s (PID existe? permissao?)\n",
                proc_path);
        return 1;
    }

    cpu_sample_t cpu_prev, cpu_now;
    io_sample_t  io_prev,  io_now;
    mem_sample_t mem_now;

    if (!read_cpu_sample(pid, &cpu_prev) ||
        !read_io_sample(pid, &io_prev)) {
        fprintf(stderr, "Falha lendo amostras iniciais de /proc.\n");
        return 1;
    }

    printf("time_ms,cpu_percent,rss_kb,vsz_kb,read_Bps,write_Bps\n");

    for (int i = 0; i < samples; ++i) {
        usleep((useconds_t)interval_ms * 1000U);

        long t_ms = now_ms();

        if (!read_cpu_sample(pid, &cpu_now) ||
            !read_io_sample(pid, &io_now)  ||
            !read_mem_sample(pid, &mem_now)) {
            fprintf(stderr, "PID %d terminou ou /proc inacessivel.\n",
                    (int)pid);
            return 1;
        }

        double cpu = cpu_usage_percent(&cpu_prev, &cpu_now);

        double dt_s = (double)interval_ms / 1000.0;
        double rbps = (io_now.read_bytes  - io_prev.read_bytes)  / dt_s;
        double wbps = (io_now.write_bytes - io_prev.write_bytes) / dt_s;

        printf("%ld,%.2f,%ld,%ld,%.0f,%.0f\n",
               t_ms, cpu,
               mem_now.rss_kb, mem_now.vsz_kb,
               rbps, wbps);

        cpu_prev = cpu_now;
        io_prev  = io_now;
    }

    return 0;
}
