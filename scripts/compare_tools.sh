#!/usr/bin/env bash
#
# compare_tools.sh
#
# Script auxiliar para comparar o resource_monitor com uma ferramenta
# de referência do sistema (pidstat).
#
# Uso:
#   ./scripts/compare_tools.sh <pid> <intervalo_ms> <samples>
#
# Exemplo:
#   ./scripts/compare_tools.sh 12345 500 20
#
# O script irá gerar:
#   - rm_<pid>.csv      : saída do ./resource_monitor
#   - pidstat_<pid>.txt : saída do pidstat para o mesmo processo
#
# Depois você pode abrir o CSV com scripts/visualize.py e inspecionar
# o pidstat_<pid>.txt para comparar CPU, memória e I/O.

set -euo pipefail

if [ $# -ne 3 ]; then
    echo "Uso: $0 <pid> <intervalo_ms> <samples>" >&2
    exit 1
fi

PID="$1"
INTERVAL_MS="$2"
SAMPLES="$3"

# Converte intervalo_ms para segundos inteiros para o pidstat
# (pidstat trabalha em segundos).
if [ "$INTERVAL_MS" -lt 1000 ]; then
    INTERVAL_S=1
else
    INTERVAL_S=$(( INTERVAL_MS / 1000 ))
fi

RM_OUT="rm_${PID}.csv"
PIDSTAT_OUT="pidstat_${PID}.txt"

echo "==> Gerando saida do resource_monitor em ${RM_OUT} ..."
./resource_monitor "$PID" "$INTERVAL_MS" "$SAMPLES" > "$RM_OUT"

echo "==> Gerando saida do pidstat em ${PIDSTAT_OUT} ..."
if command -v pidstat >/dev/null 2>&1; then
    # -r : memoria, -d : I/O, -u : CPU, -h : formato legivel
    pidstat -h -r -d -u -p "$PID" "$INTERVAL_S" "$SAMPLES" > "$PIDSTAT_OUT"
else
    echo "Aviso: pidstat nao encontrado no sistema." >&2
    echo "Instale o pacote 'sysstat' para comparacao detalhada." >&2
fi

echo
echo "Arquivos gerados:"
echo "  - ${RM_OUT}"
echo "  - ${PIDSTAT_OUT} (se pidstat estiver instalado)"
echo
echo "Voce pode visualizar o CSV com:"
echo "  python3 scripts/visualize.py ${RM_OUT}"
