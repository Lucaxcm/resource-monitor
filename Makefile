# Compilação padrão
CC      = cc
CFLAGS  = -std=c11 -O2 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS =

# Fontes do resource profiler
SRC_MON = src/main.c src/cpu_monitor.c src/memory_monitor.c src/io_monitor.c

# Fontes do namespace analyzer
SRC_NS  = src/namespace_analyzer.c

# Fontes do cgroup manager
SRC_CG  = src/cgroup.c src/cgroup_manager.c

# Binários de teste
TESTS = tests/test_cpu tests/test_memory tests/test_io

# Alvo padrão: tudo
all: resource_monitor ns_analyzer cg_manager

# ------------- Binários principais -------------

resource_monitor: $(SRC_MON)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

ns_analyzer: $(SRC_NS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cg_manager: $(SRC_CG)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# ------------- Programas de teste -------------

tests/test_cpu: tests/test_cpu.c
	$(CC) $(CFLAGS) -o $@ $^

tests/test_memory: tests/test_memory.c
	$(CC) $(CFLAGS) -o $@ $^

tests/test_io: tests/test_io.c
	$(CC) $(CFLAGS) -o $@ $^

tests: $(TESTS)

# ------------- Limpeza -------------

clean:
	rm -f resource_monitor ns_analyzer cg_manager
	rm -f $(TESTS)

.PHONY: all tests clean

