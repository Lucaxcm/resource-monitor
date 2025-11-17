#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void pause_enter(void){ printf("\n(Enter para continuar) "); getchar(); }

int main(void){
  for(;;){
    printf("\n===== RA3 MENU =====\n");
    printf("1) Resource Monitor (PID)\n");
    printf("2) Namespace Analyzer\n");
    printf("3) CGroup Manager\n");
    printf("0) Sair\n> ");
    int op=0;
    if (scanf("%d",&op)!=1) { return 0; }
    getchar();  // consome '\n'

    if(op==0) break;

    else if(op==1){
      int pid=0, ms=0, n=0;
      if (printf("PID: ")<0 || scanf("%d",&pid)!=1) { puts("Entrada inválida"); getchar(); continue; }
      if (printf("Intervalo (ms): ")<0 || scanf("%d",&ms)!=1) { puts("Entrada inválida"); getchar(); continue; }
      if (printf("Samples: ")<0 || scanf("%d",&n)!=1) { puts("Entrada inválida"); getchar(); continue; }
      getchar();
      char cmd[256];
      snprintf(cmd,sizeof(cmd),"./resource_monitor --pid %d --interval %d --samples %d",pid,ms,n);
      int rc = system(cmd); (void)rc;
      pause_enter();
    }

    else if(op==2){
      printf("\n[Namespace]\n1) list --pid\n2) compare --pid A --pid2 B\n3) map --type\n4) overhead --flags --runs\n> ");
      int o=0; if (scanf("%d",&o)!=1){ puts("Entrada inválida"); getchar(); continue; } getchar();
      char cmd[256];

      if(o==1){
        int pid=0; printf("PID: ");
        if (scanf("%d",&pid)!=1){ puts("Entrada inválida"); getchar(); continue; }
        getchar();
        snprintf(cmd,sizeof(cmd),"./ns_analyzer list --pid %d",pid);
      } else if(o==2){
        int a=0,b=0;
        printf("PID A: "); if (scanf("%d",&a)!=1){ puts("Entrada inválida"); getchar(); continue; }
        printf("PID B: "); if (scanf("%d",&b)!=1){ puts("Entrada inválida"); getchar(); continue; }
        getchar();
        snprintf(cmd,sizeof(cmd),"./ns_analyzer compare --pid %d --pid2 %d",a,b);
      } else if(o==3){
        char t[32];
        printf("type (mnt|uts|ipc|pid|user|net|cgroup): ");
        if (scanf("%31s",t)!=1){ puts("Entrada inválida"); getchar(); continue; }
        getchar();
        snprintf(cmd,sizeof(cmd),"./ns_analyzer map --type %s",t);
      } else {
        char f[64]; int runs=0;
        printf("flags csv: "); if (scanf("%63s",f)!=1){ puts("Entrada inválida"); getchar(); continue; }
        printf("runs: "); if (scanf("%d",&runs)!=1){ puts("Entrada inválida"); getchar(); continue; }
        getchar();
        snprintf(cmd,sizeof(cmd),"./ns_analyzer overhead --flags %s --runs %d",f,runs);
      }
      int rc = system(cmd); (void)rc;
      pause_enter();
    }

    else if(op==3){
      printf("\n[CGroup]\n1) create <name>\n2) move --self\n3) cpu --max\n4) mem --max\n5) pids --max\n6) show\n> ");
      int o=0; if (scanf("%d",&o)!=1){ puts("Entrada inválida"); getchar(); continue; } getchar();
      char name[64], arg[128], cmd[256];
      printf("name: "); if (scanf("%63s",name)!=1){ puts("Entrada inválida"); getchar(); continue; } getchar();

      if(o==1){
        snprintf(cmd,sizeof(cmd),"sudo ./cg_manager create %s",name);
      } else if(o==2){
        snprintf(cmd,sizeof(cmd),"sudo ./cg_manager move %s --self",name);
      } else if(o==3){
        printf("QUOTA PERIOD (ex 25000 100000): ");
        if (!fgets(arg,sizeof(arg),stdin)){ puts("Entrada inválida"); continue; }
        arg[strcspn(arg,"\n")]=0;
        snprintf(cmd,sizeof(cmd),"sudo ./cg_manager cpu %s --max \"%s\"",name,arg);
      } else if(o==4){
        printf("mem max (ex 200M): ");
        if (!fgets(arg,sizeof(arg),stdin)){ puts("Entrada inválida"); continue; }
        arg[strcspn(arg,"\n")]=0;
        snprintf(cmd,sizeof(cmd),"sudo ./cg_manager mem %s --max %s",name,arg);
      } else if(o==5){
        printf("pids.max (N ou max): ");
        if (!fgets(arg,sizeof(arg),stdin)){ puts("Entrada inválida"); continue; }
        arg[strcspn(arg,"\n")]=0;
        snprintf(cmd,sizeof(cmd),"sudo ./cg_manager pids %s --max %s",name,arg);
      } else {
        snprintf(cmd,sizeof(cmd),"sudo ./cg_manager show %s",name);
      }
      int rc = system(cmd); (void)rc;
      pause_enter();
    }
  }
  return 0;
}
