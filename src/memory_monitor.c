#include "monitor.h"
#include <stdio.h>
#include <string.h>

static long long kb_to_bytes(long long kb){ return kb * 1024LL; }

bool read_mem_status(int pid, mem_sample_t* out){
  char path[64]; snprintf(path,sizeof(path),"/proc/%d/status",pid);
  FILE* f=fopen(path,"r"); if(!f) return false;

  long long vmrss_kb=-1, vmsize_kb=-1, vmswap_kb=-1;
  char key[64], unit[16]; long long val=0;

  while (fscanf(f,"%63s",key)==1){
    if(strcmp(key,"VmRSS:")==0){
      int rc = fscanf(f,"%lld %15s",&val,unit); (void)rc;
      vmrss_kb = val;
    } else if(strcmp(key,"VmSize:")==0){
      int rc = fscanf(f,"%lld %15s",&val,unit); (void)rc;
      vmsize_kb = val;
    } else if(strcmp(key,"VmSwap:")==0){
      int rc = fscanf(f,"%lld %15s",&val,unit); (void)rc;
      vmswap_kb = val;
    } else {
      int c; while((c=fgetc(f))!='\n' && c!=EOF){}  // descarta resto
    }
  }
  fclose(f);

  out->rss_bytes   = (vmrss_kb>=0)? kb_to_bytes(vmrss_kb)  : -1;
  out->vms_bytes   = (vmsize_kb>=0)? kb_to_bytes(vmsize_kb) : -1;
  out->vswap_bytes = (vmswap_kb>=0)? kb_to_bytes(vmswap_kb) : 0;
  return (vmrss_kb>=0 || vmsize_kb>=0);
}
