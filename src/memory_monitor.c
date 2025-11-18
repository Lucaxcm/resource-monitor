#include "monitor.h"
#include <stdio.h>
#include <string.h>

/*
 * Lê /proc/<pid>/status e procura as linhas:
 *   VmRSS:  <valor> kB
 *   VmSize: <valor> kB
 * Usamos fgets + sscanf linha a linha porque o formato do arquivo
 * não é fixo para todas as linhas.
 */

bool read_mem_sample(pid_t pid, mem_sample_t *out) {
    if (!out) return false;

    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/status", (int)pid);

    FILE *f = fopen(path, "r");
    if (!f) return false;

    long rss = -1;
    long vsz = -1;
    char line[256];

    while (fgets(line, sizeof(line), f)) {
        long value;

        /* tenta casar com "VmRSS:" */
        if (sscanf(line, "VmRSS: %ld", &value) == 1) {
            rss = value;
        }
        /* tenta casar com "VmSize:" */
        else if (sscanf(line, "VmSize: %ld", &value) == 1) {
            vsz = value;
        }
    }

    fclose(f);

    if (rss < 0 || vsz < 0) {
        /* não conseguimos encontrar uma das métricas */
        return false;
    }

    out->rss_kb = rss;
    out->vsz_kb = vsz;
    return true;
}
