# ARCHITECTURE.md

# Arquitetura do Projeto – Resource Monitor (RA3)

Este documento descreve exclusivamente **a organização interna**, **responsabilidades dos módulos** e **fluxos arquiteturais** do projeto.  
Aqui **não há instruções de uso** — isso pertence ao README.md.

---

# 1. Objetivo da Arquitetura

A arquitetura foi projetada para ser:

- **Modular** — cada parte do sistema é isolada.  
- **Enxuta** — apenas o que a RA3 exige.  
- **Clara** — separação entre monitoramento, namespaces e cgroups.  
- **Extensível** — módulos independentes podem crescer sem afetar o resto.

---

# 2. Estrutura Geral do Projeto

```
resource-monitor/
├── include/        # Headers (interfaces)
├── src/            # Implementações principais
├── tests/          # Programas geradores de carga
└── scripts/        # Visualização e comparação
```

Cada diretório cumpre um papel específico na arquitetura.

---

# 3. Biblioteca de Monitoramento (`include/monitor.h` + `src/*monitor*`)

Este módulo contém **toda a lógica de acesso ao /proc**, separado em três arquivos:

### 3.1 `cpu_monitor.c`
- Lê `/proc/<pid>/stat` e `/proc/stat`
- Calcula jiffies do processo e totais do sistema

### 3.2 `memory_monitor.c`
- Lê `/proc/<pid>/status`
- Extrai valores `VmRSS` e `VmSize`

### 3.3 `io_monitor.c`
- Lê `/proc/<pid>/io`
- Extrai `read_bytes` e `write_bytes`

### 3.4 Papel arquitetural
- Fornecer medições encapsuladas  
- Permitir que o binário `resource_monitor` apenas coordene o fluxo

---

# 4. Resource Profiler (`resource_monitor`)

### Arquivos envolvidos
- `src/main.c`
- `cpu_monitor.c`
- `memory_monitor.c`
- `io_monitor.c`

### Função arquitetural
Coordenar:
1. Leitura de amostras  
2. Cálculo de diferenças  
3. Escrita de CSV  
4. Controle de loop  
5. Detecção de término do processo

### Fluxo interno
1. Validação inicial  
2. Leitura das amostras iniciais  
3. Loop de amostragem  
4. Escrita do CSV  
5. Encerramento seguro  

Não acessa diretamente `/proc` — delega tudo aos módulos especialistas.

---

# 5. Namespace Analyzer (`src/namespace_analyzer.c`)

### Arquitetura
Define o binário **ns_analyzer** usando:

- `include/namespace.h`
- leitura dos links `/proc/<pid>/ns/*`

### Responsabilidades
- Extrair IDs de namespace  
- Comparar dois processos  
- Gerar relatório percorrendo `/proc`  

### Papel arquitetural
Permite explorar **isolamento de processos**, independente dos outros módulos.

---

# 6. Control Group Manager (`cg_manager`)

### Arquivos envolvidos
- `include/cgroup.h`
- `src/cgroup.c`
- `src/cgroup_manager.c`

### Funções
- Criar diretórios de cgroup v2  
- Mover processos para dentro de um cgroup  
- Ajustar limites de CPU e memória  
- Ler estatísticas (`cpu.stat`, `memory.current`)

### Papel arquitetural
É o módulo de **controle** do projeto, complementando o monitor.

---

# 7. Programas de Teste (`tests/`)

São ferramentas internas usadas para avaliar a precisão do `resource_monitor`.

- `test_cpu.c` → carga de CPU  
- `test_memory.c` → carga de memória  
- `test_io.c` → carga de disco  

### Papel arquitetural
- Permitem replicar cenários previsíveis  
- Não influenciam o funcionamento do sistema principal  
- São desacoplados — nenhum módulo depende deles

---

# 8. Scripts (`scripts/`)

### 8.1 `visualize.py`
- Lê CSVs do monitor  
- Calcula estatísticas  
- Plota gráficos (se houver matplotlib)

### 8.2 `compare_tools.sh`
- Executa `resource_monitor` + `pidstat` simultaneamente  
- Produz arquivos comparativos

### Papel arquitetural
Camada **auxiliar**, voltada à análise externa.

---

# 9. Independência entre Módulos

| Módulo | Depende de | Independente de |
|-------|------------|-----------------|
| resource_monitor | monitor.h | namespaces, cgroups |
| ns_analyzer | namespace.h | monitor, cgroups |
| cg_manager | cgroup.h | monitor, namespaces |
| tests | nada | tudo |
| scripts | CSV e pidstat | código interno |

Cada componente pode ser modificado sem quebrar os outros.

---

# 10. Filosofia Arquitetural

- **Separação total de responsabilidades**  
- **Acoplamento mínimo**  
- **Modularidade máxima**  
- **Código limpo e direto ao ponto**  
- **Focado no que a RA3 exige, sem excessos**

---

# 11. Conclusão

A arquitetura foi construída para ser:

- fácil de entender  
- fácil de testar  
- fácil de manter  
- fiel ao modelo modular ensinado em Sistemas Operacionais

Cada módulo cobre uma parte clara da RA3:  
monitoramento, isolamento (namespaces) e controle de recursos (cgroups).  
Tudo isso formando um projeto completo, organizado e didático.
