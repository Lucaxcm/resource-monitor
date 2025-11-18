// tests/test_cpu.c
//
// Programa de teste CPU-intensive.
// Gera carga de CPU em um único processo por um tempo configurável.
//
// Uso:
//   ./test_cpu [segundos]
//
// Exemplo:
//   ./test_cpu 30
//
// Depois, em outro terminal, você pode rodar o resource_monitor nesse PID.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>  // getpid

int main(int argc, char **argv) {
    int duration = 30; /* duração padrão em segundos */

    if (argc >= 2) {
        duration = atoi(argv[1]);
        if (duration <= 0) {
            duration = 30;
        }
    }

    printf("test_cpu: PID=%d, rodando por %d segundos...\n",
           (int)getpid(), duration);

    time_t end = time(NULL) + duration;

    /* variável volátil para evitar otimizações agressivas do compilador */
    volatile double x = 0.0;

    while (time(NULL) < end) {
        /* loop apertado com algumas operações de ponto flutuante */
        for (int i = 0; i < 1000000; ++i) {
            x += (double)i * 0.0000001;
        }

        /* apenas para garantir uso de x */
        if (x > 1e9) {
            x = 0.0;
        }
    }

    printf("test_cpu: terminou.\n");
    return 0;
}
