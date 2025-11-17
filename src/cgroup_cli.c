#include "cgroup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void usage(void){
  fprintf(stderr,
    "uso: cg_manager <cmd> [args]\n"
    "cmds:\n"
    "  create <name>\n"
    "  move <name> --pid N | --self\n"
    "  cpu   <name> --max \"QUOTA PERIOD\"\n"
    "  mem   <name> --max BYTES\n"
    "  io    <name> --max \"MAJ:MIN rbps=.. wbps=..\"\n"
    "  pids  <name> --max N|max\n"
    "  show  <name>\n");
}

static void show(const char* name){
  char buf[4096];
  printf("== %s ==\n", name);
  if(cg_read(name,"cpu.stat",buf,sizeof(buf)))        printf("[cpu.stat]\n%s",buf);
  if(cg_read(name,"memory.current",buf,sizeof(buf)))  printf("[memory.current]\n%s",buf);
  if(cg_read(name,"memory.max",buf,sizeof(buf)))      printf("[memory.max]\n%s",buf);
  if(cg_read(name,"memory.events",buf,sizeof(buf)))   printf("[memory.events]\n%s",buf);
  if(cg_read(name,"io.stat",buf,sizeof(buf)))         printf("[io.stat]\n%s",buf);
  if(cg_read(name,"pids.current",buf,sizeof(buf)))    printf("[pids.current]\n%s",buf);
  if(cg_read(name,"pids.max",buf,sizeof(buf)))        printf("[pids.max]\n%s",buf);
}

int main(int argc,char** argv){
  if(argc<3){ usage(); return 1; }
  const char* cmd=argv[1]; const char* name=argv[2];

  if(strcmp(cmd,"create")==0){
    if(!cg_create(name)) return 2;
    puts("OK: created");
    return 0;
  }
  if(strcmp(cmd,"move")==0){
    int pid=-1,self=0;
    for(int i=3;i<argc;i++){
      if(!strcmp(argv[i],"--pid") && i+1<argc) pid=atoi(argv[++i]);
      else if(!strcmp(argv[i],"--self")) self=1;
    }
    if(self) return cg_move_self(name)?0:2;
    if(pid>0) return cg_move_pid(name,pid)?0:2;
    usage(); return 1;
  }
  if(strcmp(cmd,"cpu")==0){
    const char* s=NULL; for(int i=3;i<argc;i++) if(!strcmp(argv[i],"--max")&&i+1<argc) s=argv[++i];
    return (s && cg_set_cpu_max(name,s))?0:1;
  }
  if(strcmp(cmd,"mem")==0){
    const char* s=NULL; for(int i=3;i<argc;i++) if(!strcmp(argv[i],"--max")&&i+1<argc) s=argv[++i];
    return (s && cg_set_memory_max(name,s))?0:1;
  }
  if(strcmp(cmd,"io")==0){
    const char* s=NULL; for(int i=3;i<argc;i++) if(!strcmp(argv[i],"--max")&&i+1<argc) s=argv[++i];
    return (s && cg_set_io_max(name,s))?0:1;
  }
  if(strcmp(cmd,"pids")==0){
    const char* s=NULL; for(int i=3;i<argc;i++) if(!strcmp(argv[i],"--max")&&i+1<argc) s=argv[++i];
    return (s && cg_set_pids_max(name,s))?0:1;
  }
  if(strcmp(cmd,"show")==0){ show(name); return 0; }

  usage(); return 1;
}
