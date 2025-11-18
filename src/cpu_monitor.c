#define _GNU_SOURCE
#include "monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Soma os campos da primeira linha "cpu ..." de /proc/stat */
static int read_total_jiffies(uint64_t *total) {
    FILE *f = fopen("/proc/stat", "r");
    if (!f) return 0;

    char tag[8];
    unsigned long long v[10] = {0};

    int n = fscanf(
        f,
        "%7s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
        tag,
        &v[0], &v[1], &v[2], &v[3], &v[4],
        &v[5], &v[6], &v[7], &v[8], &v[9]
    );
    fclose(f);

    if (n < 5) return 0;

    *total = 0;
    for (int i = 0; i < n - 1; ++i) {
        *total += v[i];
    }
    return 1;
}

/* Lê utime + stime de /proc/<pid>/stat */
static int read_proc_jiffies(pid_t pid, uint64_t *out) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", (int)pid);

    FILE *f = fopen(path, "r");
    if (!f) return 0;

    char line[1024];
    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return 0;
    }
    fclose(f);

    char *p = strrchr(line, ')');
    if (!p) return 0;

    unsigned long long dummy;
    unsigned long long utime, stime;
    int scanned = sscanf(
        p + 2,
        "%*c "         /* state */
        "%llu %llu %llu %llu %llu %llu %llu %llu %llu %llu "
        "%llu %llu",   /* utime + stime */
        &dummy,&dummy,&dummy,&dummy,&dummy,
        &dummy,&dummy,&dummy,&dummy,&dummy,
        &utime,&stime
    );
    if (scanned != 12) return 0;

    *out = utime + stime;
    return 1;
}

bool read_cpu_sample(pid_t pid, cpu_sample_t *out) {
    if (!out) return false;
    uint64_t p = 0, t = 0;
    if (!read_proc_jiffies(pid, &p)) return false;
    if (!read_total_jiffies(&t))     return false;
    out->proc_jiffies  = p;
    out->total_jiffies = t;
    return true;
}

double cpu_usage_percent(const cpu_sample_t *prev,
                         const cpu_sample_t *now) {
    if (!prev || !now) return 0.0;

    long ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    if (ncpu <= 0) ncpu = 1;

    double dproc = (double)(now->proc_jiffies  - prev->proc_jiffies);
    double dtot  = (double)(now->total_jiffies - prev->total_jiffies);

    if (dproc < 0.0 || dtot <= 0.0) return 0.0;

    return (dproc / dtot) * 100.0 * (double)ncpu;
}
