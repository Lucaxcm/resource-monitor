// tests/test_memory.c
//
// Programa de teste memory-intensive.
// Aloca um buffer grande em memória e fica "passeando" por ele,
// gerando uso de RAM e algum acesso ao barramento.
//
// Uso:
//   ./test_memory [megas] [segundos]
//
// Exemplo (500 MB por 60 s):
//   ./test_memory 500 60

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
    int mb       = 500; /* tamanho padrão: 500 MB */
    int duration = 60;  /* duração padrão: 60 s   */

    if (argc >= 2) {
        mb = atoi(argv[1]);
        if (mb <= 0) mb = 500;
    }
    if (argc >= 3) {
        duration = atoi(argv[2]);
        if (duration <= 0) duration = 60;
    }

    size_t bytes = (size_t)mb * 1024 * 1024;

    printf("test_memory: alocando %d MB por %d segundos...\n",
           mb, duration);

    char *buffer = malloc(bytes);
    if (!buffer) {
        perror("malloc");
        return 1;
    }

    /* inicializa a memória para garantir mapeamento das páginas */
    for (size_t i = 0; i < bytes; i += 4096) {
        buffer[i] = (char)(i & 0xFF);
    }

    time_t end = time(NULL) + duration;

    while (time(NULL) < end) {
        /* toca periodicamente nas páginas para mantê-las “quentes” */
        for (size_t i = 0; i < bytes; i += 4096) {
            buffer[i]++; /* escrita leve em cada página */
        }
        /* sem usleep: gera ainda mais atividade de memória */
    }

    free(buffer);
    printf("test_memory: terminou.\n");
    return 0;
}
