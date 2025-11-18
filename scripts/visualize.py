#!/usr/bin/env python3
"""
visualize.py

Script auxiliar para visualizar a saída do resource_monitor.

Uso:
    python3 scripts/visualize.py dados.csv

Onde dados.csv é um arquivo gerado por:
    ./resource_monitor <pid> <intervalo_ms> <samples> > dados.csv

Formato esperado do CSV:
    time_ms,cpu_percent,rss_kb,vsz_kb,read_Bps,write_Bps
"""

import csv
import sys
from statistics import mean

# Tentamos usar matplotlib para gerar gráficos.
# Se não estiver instalado, o script ainda funciona mostrando estatísticas
# numéricas no terminal.
try:
    import matplotlib.pyplot as plt
    HAVE_MPL = True
except ImportError:
    HAVE_MPL = False


def load_csv(path):
    """Lê o CSV do resource_monitor e devolve um dicionário de listas."""
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        times_ms = []
        cpu = []
        rss = []
        vsz = []
        rbps = []
        wbps = []

        for row in reader:
            # Converte strings em números, ignorando linhas inválidas
            try:
                times_ms.append(float(row["time_ms"]))
                cpu.append(float(row["cpu_percent"]))
                rss.append(float(row["rss_kb"]))
                vsz.append(float(row["vsz_kb"]))
                rbps.append(float(row["read_Bps"]))
                wbps.append(float(row["write_Bps"]))
            except (KeyError, ValueError):
                continue

    return {
        "time_ms": times_ms,
        "cpu": cpu,
        "rss": rss,
        "vsz": vsz,
        "rbps": rbps,
        "wbps": wbps,
    }


def print_stats(data):
    """Imprime algumas estatísticas simples das séries."""
    def fmt_stats(name, values):
        if not values:
            print(f"{name}: sem dados")
            return
        print(
            f"{name}: min={min(values):.2f}, "
            f"max={max(values):.2f}, "
            f"media={mean(values):.2f}"
        )

    print("=== Estatisticas do arquivo ===")
    fmt_stats("CPU (%)", data["cpu"])
    fmt_stats("RSS (kB)", data["rss"])
    fmt_stats("VSZ (kB)", data["vsz"])
    fmt_stats("Read Bps", data["rbps"])
    fmt_stats("Write Bps", data["wbps"])
    print("================================")


def plot_data(data):
    """Plota as métricas ao longo do tempo (se matplotlib estiver disponível)."""
    if not HAVE_MPL:
        print("matplotlib nao encontrado; mostrando apenas estatisticas numericas.")
        return

    if not data["time_ms"]:
        print("Sem dados para plotar.")
        return

    # Normaliza tempo para segundos desde o primeiro ponto
    t0 = data["time_ms"][0]
    t = [(x - t0) / 1000.0 for x in data["time_ms"]]

    # Um gráfico por métrica para ficar simples
    plt.figure()
    plt.plot(t, data["cpu"])
    plt.xlabel("Tempo (s)")
    plt.ylabel("CPU (%)")
    plt.title("Uso de CPU ao longo do tempo")

    plt.figure()
    plt.plot(t, data["rss"])
    plt.xlabel("Tempo (s)")
    plt.ylabel("RSS (kB)")
    plt.title("Memoria residente (RSS)")

    plt.figure()
    plt.plot(t, data["rbps"], label="read_Bps")
    plt.plot(t, data["wbps"], label="write_Bps")
    plt.xlabel("Tempo (s)")
    plt.ylabel("Bytes por segundo")
    plt.title("Taxas de I/O")
    plt.legend()

    plt.show()


def main():
    if len(sys.argv) != 2:
        print("Uso: python3 scripts/visualize.py <arquivo.csv>")
        sys.exit(1)

    path = sys.argv[1]
    data = load_csv(path)
    print_stats(data)
    plot_data(data)


if __name__ == "__main__":
    main()
