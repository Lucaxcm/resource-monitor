// tests/test_io.c
//
// Programa de teste I/O-intensive.
// Escreve um arquivo temporário em disco em blocos pequenos,
// gerando bastante atividade de I/O.
//
// Uso:
//   ./test_io [megas] [bloco_kb]
//
// Exemplo (escreve 500 MB em blocos de 4 KB):
//   ./test_io 500 4

#include <stdio.h>    // FILE, fopen, fwrite, fflush, printf, perror
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    int mb        = 500; /* total de MB a escrever */
    int block_kb  = 4;   /* tamanho de cada bloco em KB */
    const char *filename = "test_io.tmp";

    if (argc >= 2) {
        mb = atoi(argv[1]);
        if (mb <= 0) mb = 500;
    }
    if (argc >= 3) {
        block_kb = atoi(argv[2]);
        if (block_kb <= 0) block_kb = 4;
    }

    size_t total_bytes = (size_t)mb * 1024 * 1024;
    size_t block_size  = (size_t)block_kb * 1024;

    printf("test_io: escrevendo %d MB em blocos de %d KB no arquivo %s...\n",
           mb, block_kb, filename);

    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    char *buffer = malloc(block_size);
    if (!buffer) {
        perror("malloc");
        fclose(f);
        return 1;
    }
    memset(buffer, 'A', block_size);

    size_t written = 0;
    while (written < total_bytes) {
        size_t to_write = block_size;
        if (total_bytes - written < block_size) {
            to_write = total_bytes - written;
        }

        if (fwrite(buffer, 1, to_write, f) != to_write) {
            perror("fwrite");
            free(buffer);
            fclose(f);
            return 1;
        }
        written += to_write;

        /* flush periódico (sem fsync/fileno, só stdio puro) */
        if (written % (10 * 1024 * 1024) == 0) { /* a cada ~10MB */
            fflush(f);
        }
    }

    fflush(f);
    fclose(f);
    free(buffer);

    printf("test_io: terminou. Arquivo %s pode ser removido apos o teste.\n",
           filename);

    return 0;
}
