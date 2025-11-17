#define _GNU_SOURCE
#include "monitor.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>   // PATH_MAX

/* conta linhas (menos cabeçalho) */
static int count_entries(const char* path, int header_lines){
  FILE* f=fopen(path,"r"); if(!f) return 0;
  char line[512]; int n=0, skip=header_lines;
  while(fgets(line,sizeof(line),f)){
    if(skip>0){ skip--; continue; }
    if(line[0]=='\n' || line[0]==0) continue;
    n++;
  }
  fclose(f);
  return n;
}

/* sockets em /proc/<pid>/fd */
static int count_fd_sockets(int pid){
  char dirp[64]; snprintf(dirp,sizeof(dirp),"/proc/%d/fd",pid);
  DIR* d=opendir(dirp); if(!d) return 0;
  int c=0; struct dirent* e; char target[128];
  while((e=readdir(d))){
    if(e->d_name[0]=='.') continue;
    char p[PATH_MAX];
    int n = snprintf(p,sizeof(p),"%s/%s",dirp,e->d_name);
    if(n < 0 || (size_t)n >= sizeof(p)) continue; // evita overflow
    ssize_t r=readlink(p,target,sizeof(target)-1);
    if(r>0){
      target[r]=0;
      if(strncmp(target,"socket:[",8)==0) c++;
    }
  }
  closedir(d);
  return c;
}

/* pega Tcp InSegs/OutSegs e Udp In/Out de /proc/<pid>/net/snmp */
static void read_snmp_counts(int pid, unsigned long long* inS, unsigned long long* outS,
                             unsigned long long* inD, unsigned long long* outD){
  if(inS)  *inS  = 0;
  if(outS) *outS = 0;
  if(inD)  *inD  = 0;
  if(outD) *outD = 0;

  char path[64]; snprintf(path,sizeof(path),"/proc/%d/net/snmp",pid);
  FILE* f=fopen(path,"r"); if(!f) return;

  char header[1024], values[1024];
  while(fgets(header,sizeof(header),f)){
    if(strncmp(header,"Tcp:",4)==0){
      if(!fgets(values,sizeof(values),f)) break;
      unsigned long long a,b,c,d,e,f1,g,h,curr,inSegs,outSegs;
      int rc = sscanf(values,"Tcp: %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                      &a,&b,&c,&d,&e,&f1,&g,&h,&curr,&inSegs,&outSegs);
      if(rc>=11){ if(inS) *inS=inSegs; if(outS) *outS=outSegs; }
    } else if(strncmp(header,"Udp:",4)==0){
      if(!fgets(values,sizeof(values),f)) break;
      unsigned long long inDat=0,outDat=0;
      int rc = sscanf(values,"Udp: %llu %llu",&inDat,&outDat);
      if(rc>=2){ if(inD) *inD=inDat; if(outD) *outD=outDat; }
    }
  }
  fclose(f);
}

bool read_proc_net_counts(int pid, net_proc_t* out){
  memset(out,0,sizeof(*out));
  char p[64];
  snprintf(p,sizeof(p),"/proc/%d/net/tcp",pid);  out->tcp  = count_entries(p,1);
  snprintf(p,sizeof(p),"/proc/%d/net/tcp6",pid); out->tcp6 = count_entries(p,1);
  snprintf(p,sizeof(p),"/proc/%d/net/udp",pid);  out->udp  = count_entries(p,1);
  snprintf(p,sizeof(p),"/proc/%d/net/udp6",pid); out->udp6 = count_entries(p,1);
  snprintf(p,sizeof(p),"/proc/%d/net/raw",pid);  out->raw  = count_entries(p,1);
  snprintf(p,sizeof(p),"/proc/%d/net/raw6",pid); out->raw6 = count_entries(p,1);
  snprintf(p,sizeof(p),"/proc/%d/net/unix",pid); out->unix_sockets = count_entries(p,1);
  out->fd_sockets = count_fd_sockets(pid);

  read_snmp_counts(pid,&out->ns_in_segs,&out->ns_out_segs,&out->ns_in_dgrams,&out->ns_out_dgrams);
  return true;
}
