#define _GNU_SOURCE
#include "monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* minflt/majflt e num_threads de /proc/<pid>/stat */
static int read_stat_faults_threads(int pid, unsigned long long* minf, unsigned long long* majf, int* threads){
  char path[64]; snprintf(path,sizeof(path),"/proc/%d/stat",pid);
  FILE* f=fopen(path,"r"); if(!f) return 0;
  char* line=NULL; size_t cap=0; ssize_t len=getline(&line,&cap,f);
  fclose(f); if(len<=0){ free(line); return 0; }

  char* rparen=strrchr(line,')'); if(!rparen){ free(line); return 0; }
  char* rest=rparen+2;

  int idx=1;
  unsigned long long _min=0,_maj=0; int _thr=0;
  char* save=NULL; char* tok=strtok_r(rest," ",&save);
  while(tok){
    if(idx==7)   _min=strtoull(tok,NULL,10);     /* campo 10 */
    else if(idx==9)  _maj=strtoull(tok,NULL,10); /* campo 12 */
    else if(idx==18) _thr=atoi(tok);             /* campo 20 */
    tok=strtok_r(NULL," ",&save); idx++;
    if(idx>19 && _min && _maj && _thr) break;
  }
  free(line);

  if(minf)    *minf=_min;
  if(majf)    *majf=_maj;
  if(threads) *threads=_thr;
  return 1;
}

/* ctx switches / threads / swap de /proc/<pid>/status */
static void read_status_cs_threads(int pid, long long* vcs, long long* nvcs, int* threads, long long* vswap){
  char path[64]; snprintf(path,sizeof(path),"/proc/%d/status",pid);
  FILE* f=fopen(path,"r"); if(!f) return;
  char key[64], unit[16]; long long val=0;

  while(fscanf(f,"%63s",key)==1){
    if(strcmp(key,"voluntary_ctxt_switches:")==0){
      int rc=fscanf(f,"%lld",&val); (void)rc; if(vcs) *vcs=val;
    } else if(strcmp(key,"nonvoluntary_ctxt_switches:")==0){
      int rc=fscanf(f,"%lld",&val); (void)rc; if(nvcs) *nvcs=val;
    } else if(strcmp(key,"Threads:")==0){
      int rc=fscanf(f,"%lld",&val); (void)rc; if(threads) *threads=(int)val;
    } else if(strcmp(key,"VmSwap:")==0){
      int rc=fscanf(f,"%lld %15s",&val,unit); (void)rc; if(vswap) *vswap=val*1024LL;
    } else {
      int c; while((c=fgetc(f))!='\n' && c!=EOF){}
    }
  }
  fclose(f);
}

bool read_proc_extras(int pid, proc_extra_t* out){
  if(!out) return false;
  memset(out,0,sizeof(*out));
  long long vcs=-1,nvcs=-1,vswap=-1; int th=-1;
  read_status_cs_threads(pid,&vcs,&nvcs,&th,&vswap);
  unsigned long long minf=0,majf=0; int th2=0;
  read_stat_faults_threads(pid,&minf,&majf,&th2);
  out->voluntary_cs   = (vcs>=0)?vcs:0;
  out->nonvoluntary_cs= (nvcs>=0)?nvcs:0;
  out->threads        = th>0?th:th2;
  out->minflt         = minf;
  out->majflt         = majf;
  return true;
}
