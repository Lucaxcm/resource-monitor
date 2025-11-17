# Namespace Analyzer – Experimentos

## Ambiente
- Distro/Kernel: `uname -a`
- WSL2 + systemd: OK
- Observação: em WSL o `net` namespace costuma ser compartilhado e `CLONE_NEWNET` pode falhar (EPERM).

## 1) Listar namespaces de um PID
Comando: `./ns_analyzer list --pid $$`
Resultado resumido: inodes para mnt/uts/ipc/pid/user/net/cgroup.

## 2) Comparar dois PIDs
Comando: `./ns_analyzer compare --pid <pid1> --pid2 <pid2>`
Resumo: inodes iguais ⇒ mesmos namespaces (sem isolamento); se diferentes, indicar quais.

## 3) Mapear processos por tipo
Comando: `./ns_analyzer map --type mnt`
Resumo: grupos (inode → PIDs) e interpretação (maioria no mesmo grupo, poucos isolados).

## 4) Overhead de criação (unshare)
Comandos:
- `./ns_analyzer overhead --flags user --runs 50`
- `./ns_analyzer overhead --flags user,uts --runs 50`

Tabela (exemplo):
| Flags       | Runs | OK | Fail | Média (µs) |
|-------------|------|----|------|------------|
| user        | 50   | .. | ..   | ..         |
| user,uts    | 50   | .. | ..   | ..         |

### Conclusões
- Valores típicos de `user`-ns são dezenas de microssegundos.
- Combinações com `net` podem falhar no WSL.
