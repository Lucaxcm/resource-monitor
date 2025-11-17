#define _GNU_SOURCE
#include "monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void usage(void){
  fprintf(stderr,"uso: ./resource_monitor --pid <PID> [--interval <ms>] [--samples <N>]\n");
}
static long long now_ms(void){ struct timespec ts; clock_gettime(CLOCK_REALTIME,&ts); return (long long)ts.tv_sec*1000LL + ts.tv_nsec/1000000LL; }

int main(int argc, char** argv){
  int pid=-1, interval_ms=500, samples=10;
  for(int i=1;i<argc;i++){
    if(!strcmp(argv[i],"--pid")&&i+1<argc) pid=atoi(argv[++i]);
    else if(!strcmp(argv[i],"--interval")&&i+1<argc) interval_ms=atoi(argv[++i]);
    else if(!strcmp(argv[i],"--samples")&&i+1<argc) samples=atoi(argv[++i]);
  }
  if(pid<=0){ usage(); return 1; }

  cpu_times_t prev_cpu; int ok=0;
  for(int t=0;t<50;t++){ if(read_cpu_times(pid,&prev_cpu)){ ok=1; break; } usleep(10000); }
  if(!ok){ fprintf(stderr,"erro: não consegui ler CPU do PID %d\n", pid); return 2; }

  mem_sample_t prev_mem={0}; read_mem_status(pid,&prev_mem);
  io_cum_t     prev_io={0};  read_proc_io_totals(pid,&prev_io);
  net_proc_t   prev_net={0}; read_proc_net_counts(pid,&prev_net);
  proc_extra_t prev_x={0};   read_proc_extras(pid,&prev_x);

  printf("ts_ms,cpu_pct,rss_bytes,vms_bytes,vswap_bytes,"
         "rss_rate_bps,"
         "read_bps,write_bps,rchar_bps,wchar_bps,read_syscps,write_syscps,"
         "ctx_v,ctx_nv,threads,minflt_ps,majflt_ps,"
         "tcp,tcp6,udp,udp6,raw,raw6,unix,fd_socks,"
         "ns_in_segs_ps,ns_out_segs_ps,ns_in_dgrams_ps,ns_out_dgrams_ps\n");

  for(int i=0;i<samples;i++){
    usleep(interval_ms*1000);

    cpu_times_t cur_cpu; if(!read_cpu_times(pid,&cur_cpu)) break;
    double cpu = cpu_percent(&prev_cpu,&cur_cpu);

    mem_sample_t mem={0}; read_mem_status(pid,&mem);
    double rss_rate = 0.0;
    if(prev_mem.rss_bytes>=0 && mem.rss_bytes>=0 && interval_ms>0){
      long long delta = mem.rss_bytes - prev_mem.rss_bytes;
      rss_rate = (double)delta * 1000.0 / interval_ms;
    }

    io_cum_t cur_io={0}; read_proc_io_totals(pid,&cur_io);
    double read_bps  = (double)((long long)cur_io.read_bytes  - (long long)prev_io.read_bytes ) * 1000.0 / interval_ms;
    double write_bps = (double)((long long)cur_io.write_bytes - (long long)prev_io.write_bytes) * 1000.0 / interval_ms;
    double rchar_bps = (double)((long long)cur_io.rchar - (long long)prev_io.rchar) * 1000.0 / interval_ms;
    double wchar_bps = (double)((long long)cur_io.wchar - (long long)prev_io.wchar) * 1000.0 / interval_ms;
    double read_syscps  = (double)((long long)cur_io.syscr - (long long)prev_io.syscr) * 1000.0 / interval_ms;
    double write_syscps = (double)((long long)cur_io.syscw - (long long)prev_io.syscw) * 1000.0 / interval_ms;

    proc_extra_t x={0}; read_proc_extras(pid,&x);
    double minflt_ps = (double)((long long)x.minflt - (long long)prev_x.minflt) * 1000.0 / interval_ms;
    double majflt_ps = (double)((long long)x.majflt - (long long)prev_x.majflt) * 1000.0 / interval_ms;

    net_proc_t net={0}; read_proc_net_counts(pid,&net);
    double ns_in_segs_ps   = (double)((long long)net.ns_in_segs   - (long long)prev_net.ns_in_segs)   * 1000.0 / interval_ms;
    double ns_out_segs_ps  = (double)((long long)net.ns_out_segs  - (long long)prev_net.ns_out_segs)  * 1000.0 / interval_ms;
    double ns_in_dgrams_ps = (double)((long long)net.ns_in_dgrams - (long long)prev_net.ns_in_dgrams) * 1000.0 / interval_ms;
    double ns_out_dgrams_ps= (double)((long long)net.ns_out_dgrams- (long long)prev_net.ns_out_dgrams)* 1000.0 / interval_ms;

    printf("%lld,%.3f,%lld,%lld,%lld,%.0f,%.0f,%.0f,%.0f,%.0f,%.2f,%.2f,%lld,%lld,%d,%.2f,%.2f,"
           "%d,%d,%d,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,%.2f\n",
           now_ms(), cpu,
           (long long)mem.rss_bytes, (long long)mem.vms_bytes, (long long)mem.vswap_bytes,
           rss_rate, read_bps, write_bps, rchar_bps, wchar_bps, read_syscps, write_syscps,
           (long long)x.voluntary_cs,(long long)x.nonvoluntary_cs,x.threads,minflt_ps,majflt_ps,
           net.tcp,net.tcp6,net.udp,net.udp6,net.raw,net.raw6,net.unix_sockets,net.fd_sockets,
           ns_in_segs_ps,ns_out_segs_ps,ns_in_dgrams_ps,ns_out_dgrams_ps);

    prev_cpu=cur_cpu; prev_mem=mem; prev_io=cur_io; prev_x=x; prev_net=net;
  }
  return 0;
}
