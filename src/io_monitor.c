#include "monitor.h"
#include <stdio.h>
#include <string.h>

/* Lê read_bytes e write_bytes de /proc/<pid>/io */
bool read_io_sample(pid_t pid, io_sample_t *out) {
    if (!out) return false;

    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/io", (int)pid);

    FILE *f = fopen(path, "r");
    if (!f) return false;

    unsigned long long rb = 0, wb = 0;
    char key[32];
    unsigned long long value;

    while (fscanf(f, "%31[^:]: %llu\n", key, &value) == 2) {
        if (strcmp(key, "read_bytes") == 0) {
            rb = value;
        } else if (strcmp(key, "write_bytes") == 0) {
            wb = value;
        }
    }

    fclose(f);

    out->read_bytes  = rb;
    out->write_bytes = wb;
    return true;
}
