CC?=gcc
CFLAGS=-std=c11 -O2 -Wall -Wextra -Wpedantic -Iinclude
RM=rm -f

# A1+A2: profiler
RM_SRC=src/main.c src/cpu_monitor.c src/memory_monitor.c src/io_monitor.c src/net_proc.c
RM_BIN=resource_monitor

# A3: namespaces
NS_SRC=src/namespace_analyzer.c
NS_BIN=ns_analyzer

# A4: cgroups
CG_SRC=src/cgroup.c src/cgroup_cli.c
CG_BIN=cg_manager

# Menu
MENU_SRC=src/ra3_menu.c
MENU_BIN=ra3_menu

all: $(RM_BIN) $(NS_BIN) $(CG_BIN) $(MENU_BIN)

$(RM_BIN): $(RM_SRC) src/proc_extras.c
	$(CC) $(CFLAGS) -o $@ $(RM_SRC) src/proc_extras.c

$(NS_BIN): $(NS_SRC)
	$(CC) $(CFLAGS) -o $@ $(NS_SRC)

$(CG_BIN): $(CG_SRC)
	$(CC) $(CFLAGS) -o $@ $(CG_SRC)

$(MENU_BIN): $(MENU_SRC)
	$(CC) $(CFLAGS) -o $@ $(MENU_SRC)

clean:
	$(RM) $(RM_BIN) $(NS_BIN) $(CG_BIN) $(MENU_BIN)

