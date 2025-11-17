#include "monitor.h"
#include <stdio.h>
#include <string.h>

bool read_proc_io_totals(int pid, io_cum_t* out){
  char path[64]; snprintf(path,sizeof(path),"/proc/%d/io",pid);
  FILE* f=fopen(path,"r"); if(!f) return false;
  char key[64]; unsigned long long val=0;
  out->read_bytes=out->write_bytes=out->rchar=out->wchar=out->syscr=out->syscw=0;

  while (fscanf(f,"%63[^:]: %llu%*[\n]", key, &val)==2){
    if(strcmp(key,"read_bytes")==0)  out->read_bytes  = val;
    else if(strcmp(key,"write_bytes")==0) out->write_bytes = val;
    else if(strcmp(key,"rchar")==0)  out->rchar = val;
    else if(strcmp(key,"wchar")==0)  out->wchar = val;
    else if(strcmp(key,"syscr")==0)  out->syscr = val;
    else if(strcmp(key,"syscw")==0)  out->syscw = val;
  }
  fclose(f);
  return true;
}
