#ifndef MONITOR_H
#define MONITOR_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

/*
 * Estruturas e funções de monitoramento de CPU, memória e I/O
 * usadas pelo resource_monitor.
 */

/* --------------------- CPU ---------------------- */

typedef struct {
    uint64_t proc_jiffies;   /* utime + stime do processo   */
    uint64_t total_jiffies;  /* total da linha "cpu "       */
} cpu_sample_t;

/* Lê /proc/<pid>/stat e /proc/stat para montar a amostra de CPU */
bool read_cpu_sample(pid_t pid, cpu_sample_t *out);

/* Calcula o uso de CPU (%) entre duas amostras */
double cpu_usage_percent(const cpu_sample_t *prev,
                         const cpu_sample_t *now);

/* -------------------- MEMÓRIA ------------------- */

/* Amostra mínima de memória em kB */
typedef struct {
    long rss_kb;  /* memória residente (RAM) */
    long vsz_kb;  /* memória virtual total   */
} mem_sample_t;

/* Lê /proc/<pid>/status (VmRSS e VmSize) */
bool read_mem_sample(pid_t pid, mem_sample_t *out);

/* ---------------------- I/O --------------------- */

/* Amostra de I/O cumulativa em bytes */
typedef struct {
    unsigned long long read_bytes;   /* read_bytes  de /proc/<pid>/io */
    unsigned long long write_bytes;  /* write_bytes de /proc/<pid>/io */
} io_sample_t;

/* Lê /proc/<pid>/io (campos read_bytes e write_bytes) */
bool read_io_sample(pid_t pid, io_sample_t *out);

#endif /* MONITOR_H */
