#include "cgroup.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static const char* base = "/sys/fs/cgroup";

static int path(char* out, size_t n, const char* name, const char* file){
  if(file) return snprintf(out,n,"%s/%s/%s", base, name, file) < (int)n;
  return snprintf(out,n,"%s/%s", base, name) < (int)n;
}

static bool write_str(const char* name, const char* file, const char* s){
  char p[256]; if(!path(p,sizeof(p),name,file)) return false;
  FILE* f = fopen(p,"w"); if(!f){ perror(p); return false; }
  if(fprintf(f,"%s\n", s) < 0){ perror("fprintf"); fclose(f); return false; }
  fclose(f); return true;
}

bool cg_create(const char* name){
  char p[256]; if(!path(p,sizeof(p),name,NULL)) return false;
  if(mkdir(p,0755)==0) return true;
  if(errno==EEXIST) return true;
  perror("mkdir cgroup");
  return false;
}

bool cg_move_pid(const char* name, int pid){ char buf[32]; snprintf(buf,sizeof(buf),"%d", pid); return write_str(name, "cgroup.procs", buf); }
bool cg_move_self(const char* name){ return cg_move_pid(name, getpid()); }

bool cg_set_cpu_max(const char* name, const char* max_line){ return write_str(name, "cpu.max", max_line); }
bool cg_set_memory_max(const char* name, const char* bytes){ return write_str(name, "memory.max", bytes); }
bool cg_set_io_max(const char* name, const char* rule){ return write_str(name, "io.max", rule); }
bool cg_set_pids_max(const char* name, const char* n){ return write_str(name, "pids.max", n); }

bool cg_read(const char* name, const char* file, char* buf, size_t buflen){
  char p[256]; if(!path(p,sizeof(p),name,file)) return false;
  FILE* f = fopen(p,"r"); if(!f){ return false; }
  size_t r = fread(buf,1,buflen-1,f); buf[r]=0; fclose(f); return r>0;
}
