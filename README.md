Resource Monitor – RA3 Sistemas Operacionais

Projeto desenvolvido para a RA3 de Sistemas Operacionais com foco em:

1.  Resource Profiler – monitorar recursos de um processo via /proc
2.  Namespace Analyzer – inspecionar namespaces de processos
3.  Control Group Manager – gerenciar cgroups v2 pela linha de comando
4.  Programas de teste de CPU, memória e I/O
5.  Scripts para visualização e comparação com ferramentas do sistema

Todo o projeto é acionado via linha de comando, sem menus gráficos.

------------------------------------------------------------------------

1. Estrutura do Projeto

    resource-monitor/
    ├── README.md
    ├── Makefile
    ├── docs/
    │   └── ARCHITECTURE.md
    ├── include/
    │   ├── monitor.h
    │   ├── namespace.h
    │   └── cgroup.h
    ├── src/
    │   ├── main.c                # resource_monitor
    │   ├── cpu_monitor.c
    │   ├── memory_monitor.c
    │   ├── io_monitor.c
    │   ├── namespace_analyzer.c  # ns_analyzer
    │   ├── cgroup.c
    │   └── cgroup_manager.c      # cg_manager
    ├── tests/
    │   ├── test_cpu.c
    │   ├── test_memory.c
    │   └── test_io.c
    └── scripts/
        ├── visualize.py
        └── compare_tools.sh

------------------------------------------------------------------------

2. Pré-requisitos

Requer ambiente Linux com acesso a:

-   /proc (para monitoramento)
-   /sys/fs/cgroup (para cgroups v2)

Ferramentas necessárias:

-   Compilador C (ex.: gcc)
-   make
-   python3

Opcional:

-   matplotlib para gráficos no visualize.py
-   pidstat (pacote sysstat) para comparação no compare_tools.sh

------------------------------------------------------------------------

3. Compilação

Na raiz do projeto:

    make
    make tests

Para limpar:

    make clean

------------------------------------------------------------------------

4. Resource Profiler – resource_monitor

4.1 Objetivo

Monitorar:

-   CPU %
-   RSS / VSZ
-   I/O (read/write por segundo)

Saída: CSV.

4.2 Uso

    ./resource_monitor <pid> <intervalo_ms> <samples>

Exemplo:

    ./resource_monitor 12345 500 20 > teste.csv

Formato CSV:

    time_ms,cpu_percent,rss_kb,vsz_kb,read_Bps,write_Bps

------------------------------------------------------------------------

5. Programas de Teste – tests/

5.1 Teste de CPU – test_cpu

    ./tests/test_cpu 300 &

------------------------------------------------------------------------

5.2 Teste de Memória – test_memory

    ./tests/test_memory 500 60 &

------------------------------------------------------------------------

5.3 Teste de I/O – test_io

    ./tests/test_io 500 4 &

Remover arquivo:

    rm test_io.tmp

------------------------------------------------------------------------

6. Scripts – scripts/

6.1 visualize.py

    python3 scripts/visualize.py arquivo.csv

Gera estatísticas e gráficos opcionais.

------------------------------------------------------------------------

6.2 compare_tools.sh

    ./scripts/compare_tools.sh <pid> <intervalo_ms> <samples>

Gera:

-   rm_.csv
-   pidstat_.txt

------------------------------------------------------------------------

7. Namespace Analyzer – ns_analyzer

Uso:

    ./ns_analyzer list <pid>
    ./ns_analyzer compare <pid1> <pid2>
    ./ns_analyzer report

------------------------------------------------------------------------

8. Control Group Manager – cg_manager

Comandos:

    ./cg_manager create <nome>
    ./cg_manager move <nome> <pid>
    ./cg_manager self <nome>
    ./cg_manager limit-cpu <nome> "25000 100000"
    ./cg_manager limit-mem <nome> <bytes>
    ./cg_manager show <nome>

------------------------------------------------------------------------

9. Metodologia de Testes

Inclui:

-   testes sintéticos,
-   coleta via CSV,
-   análise com visualize.py,
-   comparação com pidstat,
-   isolamento com namespaces e cgroups.

------------------------------------------------------------------------

10. Observações

-   cpu_percent pode ultrapassar 100% em multicore.
-   Monitoramento é por PID único.
-   Caso o processo termine, o monitor encerra automaticamente.
