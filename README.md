# Resource Monitor – RA3 Sistemas Operacionais

Projeto desenvolvido para a RA3 de Sistemas Operacionais com foco em:

1. **Resource Profiler** – monitorar recursos de um processo via `/proc`
2. **Namespace Analyzer** – inspecionar namespaces de processos
3. **Control Group Manager** – gerenciar cgroups v2 pela linha de comando
4. **Programas de teste** de CPU, memória e I/O
5. **Scripts** para visualização e comparação com ferramentas do sistema

Todo o projeto é acionado **via linha de comando**, sem menus gráficos.

---

## 1. Estrutura do Projeto

```text
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
    
2. Pré-requisitos
Linux com acesso a:

/proc (para monitoramento)

/sys/fs/cgroup (para cgroups v2)

Compilador C (ex.: gcc)

make

python3

(Opcional) matplotlib para gráficos no visualize.py

(Opcional) pidstat (pacote sysstat) para comparação no compare_tools.sh

3. Compilação
Na raiz do projeto:

bash
Copiar código
make          # compila resource_monitor, ns_analyzer e cg_manager
make tests    # compila os programas de teste em tests/
Para limpar os binários:

bash
Copiar código
make clean
4. Resource Profiler – resource_monitor
4.1. Objetivo
Monitorar um processo (via PID) ao longo do tempo, coletando:

Uso de CPU (%)

Uso de memória (RSS e VSZ, em kB)

Taxas de I/O (bytes lidos e escritos por segundo)

A saída é um CSV, fácil de importar em planilhas ou scripts.

4.2. Uso
bash
Copiar código
./resource_monitor <pid> <intervalo_ms> <samples>
<pid> – PID do processo a monitorar

<intervalo_ms> – intervalo entre amostras em milissegundos

<samples> – número de amostras

Exemplo (20 amostras, 500 ms entre elas):

bash
Copiar código
./resource_monitor 12345 500 20 > teste.csv
O arquivo CSV gerado tem o formato:

text
Copiar código
time_ms,cpu_percent,rss_kb,vsz_kb,read_Bps,write_Bps
<... linhas com dados ...>
time_ms – timestamp em milissegundos

cpu_percent – uso de CPU (%), pode passar de 100% em multicore

rss_kb – memória residente (aprox. RAM usada)

vsz_kb – memória virtual total

read_Bps / write_Bps – bytes lidos/escritos por segundo

5. Programas de Teste – tests/
Os programas em tests/ geram cargas controladas para validar o resource_monitor.

5.1. Teste de CPU – test_cpu
Programa CPU-intensive, realiza cálculos em loop.

bash
Copiar código
./tests/test_cpu <segundos>
Exemplo (em um único terminal):

bash
Copiar código
./tests/test_cpu 300 &   # gera carga por 300 s em background
pid=$!                   # guarda o PID desse processo
./resource_monitor "$pid" 500 20 > cpu_test.csv
Comportamento esperado no CSV:

cpu_percent alto (próximo ou acima de 100%)

rss_kb e vsz_kb moderados

read_Bps e write_Bps próximos de 0

5.2. Teste de Memória – test_memory
Programa memory-intensive, aloca um grande buffer e percorre a memória.

bash
Copiar código
./tests/test_memory <megas> <segundos>
Exemplo:

bash
Copiar código
./tests/test_memory 500 60 &   # 500 MB por 60 s
pid=$!
./resource_monitor "$pid" 500 20 > mem_test.csv
Esperado:

rss_kb alto (na faixa dos MB solicitados)

vsz_kb alto

cpu_percent moderado

I/O de disco ≈ zero

5.3. Teste de I/O – test_io
Programa I/O-intensive, escreve um arquivo temporário em disco.

bash
Copiar código
./tests/test_io <megas> <bloco_kb>
Exemplo:

bash
Copiar código
./tests/test_io 500 4 &   # escreve ~500 MB em blocos de 4 KB
pid=$!
./resource_monitor "$pid" 500 20 > io_test.csv
Após o teste, o arquivo test_io.tmp pode ser removido:

bash
Copiar código
rm test_io.tmp
Esperado:

write_Bps alto enquanto o teste roda

cpu_percent moderado

rss_kb relativamente baixo

6. Scripts – scripts/
6.1. Visualização – visualize.py
Script em Python para analisar o CSV gerado pelo resource_monitor.

Uso:

bash
Copiar código
python3 scripts/visualize.py <arquivo.csv>
Exemplo:

bash
Copiar código
python3 scripts/visualize.py cpu_test.csv
O script:

Lê o CSV (time_ms,cpu_percent,rss_kb,vsz_kb,read_Bps,write_Bps)

Calcula, para cada métrica:

valor mínimo

valor máximo

média

Se matplotlib estiver instalado:

Plota gráficos de:

CPU x tempo

RSS x tempo

Read/Write Bps x tempo

Se matplotlib não estiver instalado:

Exibe apenas as estatísticas numéricas no terminal.

6.2. Comparação com pidstat – compare_tools.sh
Script em shell para comparar o resource_monitor com a ferramenta pidstat.

Uso:

bash
Copiar código
./scripts/compare_tools.sh <pid> <intervalo_ms> <samples>
Exemplo:

bash
Copiar código
./scripts/compare_tools.sh 12345 500 20
O script gera:

rm_<pid>.csv – saída do resource_monitor

pidstat_<pid>.txt – saída do pidstat (CPU, memória, I/O) para o mesmo PID

Depois, é possível analisar:

bash
Copiar código
python3 scripts/visualize.py rm_12345.csv
less pidstat_12345.txt
Se pidstat não estiver instalado, o script avisa e gera apenas o CSV.

7. Namespace Analyzer – ns_analyzer
Ferramenta para inspecionar namespaces de processos em Linux.

7.1. Uso
bash
Copiar código
./ns_analyzer list <pid>
./ns_analyzer compare <pid1> <pid2>
./ns_analyzer report
list <pid>
Lista os IDs de namespace do processo:

bash
Copiar código
./ns_analyzer list 1
Saída (exemplo):

text
Copiar código
PID 1:
  mnt    : 4026531840
  pid    : 4026531836
  uts    : 4026531838
  ipc    : 4026531839
  net    : 4026531993
  user   : 4026531837
  cgroup : 4026531835
compare <pid1> <pid2>
Compara se dois processos compartilham os mesmos namespaces (PID, NET, MNT, USER, etc.):

bash
Copiar código
./ns_analyzer compare 1234 5678
report
Gera um relatório simples de namespaces PID presentes no sistema:

bash
Copiar código
./ns_analyzer report
Mostra quantos processos existem em cada namespace de PID.

8. Control Group Manager – cg_manager
Ferramenta simples para manipular cgroups v2 em /sys/fs/cgroup.

Normalmente é necessário rodar com sudo.

8.1. Comandos
bash
Copiar código
./cg_manager create <nome>
./cg_manager move <nome> <pid>
./cg_manager self <nome>
./cg_manager limit-cpu <nome> "<linha_cpu.max>"
./cg_manager limit-mem <nome> <bytes>
./cg_manager show <nome>
Criar cgroup
bash
Copiar código
sudo ./cg_manager create teste
Cria /sys/fs/cgroup/teste (se não existir).

Mover processo
bash
Copiar código
sudo ./cg_manager move teste 12345   # move PID 12345 para o cgroup "teste"
sudo ./cg_manager self teste         # move o próprio cg_manager para o cgroup
Limitar CPU
bash
Copiar código
sudo ./cg_manager limit-cpu teste "25000 100000"
Escreve "25000 100000" em cpu.max do cgroup teste
(quota 25000, período 100000 → aproximadamente 25% de 1 CPU).

Limitar memória
bash
Copiar código
sudo ./cg_manager limit-mem teste 104857600
Limita o cgroup teste a ~100 MB (valor em bytes).

Exibir informações
bash
Copiar código
sudo ./cg_manager show teste
Tenta ler e mostrar:

cpu.max

cpu.stat

memory.max

memory.current

9. Metodologia de Testes (Resumo)
Para validar o projeto foram realizados:

Testes sintéticos com:

tests/test_cpu (CPU-intensive)

tests/test_memory (memory-intensive)

tests/test_io (I/O-intensive)

Coleta de métricas com:

resource_monitor → saída em CSV

Análise com:

scripts/visualize.py → estatísticas e (opcionalmente) gráficos

scripts/compare_tools.sh + pidstat → comparação com ferramenta de referência do sistema

Experimentos com isolamento:

ns_analyzer para comparar namespaces entre processos

cg_manager para aplicar limites de CPU/memória e observar efeito nas métricas

10. Observações
Valores de cpu_percent podem ultrapassar 100% em máquinas com múltiplos núcleos e pelo método de cálculo adotado (diferença de jiffies do processo em relação ao total do sistema).

O monitoramento é sempre de um único PID por execução.

Caso o processo monitorado termine durante a coleta, o resource_monitor detecta e encerra com uma mensagem indicando que o PID terminou ou o /proc ficou inacessível.