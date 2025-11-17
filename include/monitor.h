#ifndef MONITOR_H
#define MONITOR_H
#include <stdbool.h>
#include <stdint.h>

/* ----- CPU (para CPU%) ----- */
typedef struct {
  uint64_t proc_jiffies;   /* utime+stime do processo */
  uint64_t total_jiffies;  /* total agregado da linha "cpu " em /proc/stat */
} cpu_times_t;
bool   read_cpu_times(int pid, cpu_times_t* out);
double cpu_percent(const cpu_times_t* prev, const cpu_times_t* now);

/* ----- Extras do processo ----- */
typedef struct {
  long long voluntary_cs;      /* voluntary_ctxt_switches  */
  long long nonvoluntary_cs;   /* nonvoluntary_ctxt_switches */
  int        threads;          /* Threads */
  unsigned long long minflt;   /* page faults leves (/proc/<pid>/stat campo 10) */
  unsigned long long majflt;   /* page faults maiores (/proc/<pid>/stat campo 12) */
} proc_extra_t;
bool read_proc_extras(int pid, proc_extra_t* out);

/* ----- Memória ----- */
typedef struct {
  int64_t rss_bytes;    /* VmRSS */
  int64_t vms_bytes;    /* VmSize */
  int64_t vswap_bytes;  /* VmSwap */
} mem_sample_t;
bool read_mem_status(int pid, mem_sample_t* out);

/* ----- I/O (cumulativos por processo) ----- */
typedef struct {
  uint64_t read_bytes;   /* read_bytes  (disco)    */
  uint64_t write_bytes;  /* write_bytes (disco)    */
  uint64_t rchar;        /* bytes lidos em read()  */
  uint64_t wchar;        /* bytes escritos write() */
  uint64_t syscr;        /* qtd de read()          */
  uint64_t syscw;        /* qtd de write()         */
} io_cum_t;
bool read_proc_io_totals(int pid, io_cum_t* out);

/* ----- Rede (aprox. por processo) ----- */
/* Contadores de conexões / sockets + contadores do namespace de rede do processo */
typedef struct {
  int tcp, tcp6, udp, udp6, raw, raw6, unix_sockets, fd_sockets;
  unsigned long long ns_in_segs, ns_out_segs;         /* Tcp: InSegs/OutSegs (no netns do processo) */
  unsigned long long ns_in_dgrams, ns_out_dgrams;     /* Udp: InDatagrams/OutDatagrams (mesmo netns) */
} net_proc_t;
bool read_proc_net_counts(int pid, net_proc_t* out);

#endif
