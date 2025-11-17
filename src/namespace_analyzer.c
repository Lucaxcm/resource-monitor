#define _GNU_SOURCE
#include "namespace.h"
#include <dirent.h>
#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static const char* TYPES[] = {"mnt","uts","ipc","pid","user","net","cgroup"};
static int has_type(const char* t){
  for(size_t i=0;i<sizeof(TYPES)/sizeof(TYPES[0]);++i)
    if(strcmp(t,TYPES[i])==0) return 1;
  return 0;
}
static long long now_us(void){
  struct timespec ts; clock_gettime(CLOCK_MONOTONIC,&ts);
  return (long long)ts.tv_sec*1000000LL + ts.tv_nsec/1000LL;
}
static long long parse_inode(const char* link){ // "uts:[4026531838]"
  const char* l = strchr(link,'[');
  if(!l) return -1;
  return atoll(l+1);
}

/* ---------- list ---------- */
int ns_list_for_pid(int pid, ns_entry_t* out, int maxN){
  int n=0;
  for(size_t i=0;i<sizeof(TYPES)/sizeof(TYPES[0]) && n<maxN;i++){
    char path[64]; snprintf(path,sizeof(path),"/proc/%d/ns/%s", pid, TYPES[i]);
    ssize_t r = readlink(path, out[n].link, sizeof(out[n].link)-1);
    if(r>=0){
      out[n].link[r] = 0;
      out[n].name = TYPES[i];
      n++;
    }
  }
  return n;
}

/* ---------- compare ---------- */
int ns_compare_pids(int pidA, int pidB){
  ns_entry_t a[8], b[8];
  int na = ns_list_for_pid(pidA, a, 8);
  int nb = ns_list_for_pid(pidB, b, 8);
  if(na<=0 || nb<=0){ fprintf(stderr,"erro lendo /proc de %d ou %d\n", pidA, pidB); return -1; }

  int diff = 0;
  printf("Comparando namespaces de %d vs %d\n", pidA, pidB);
  for(int i=0;i<na;i++){
    const char* name = a[i].name;
    long long ia = parse_inode(a[i].link);
    long long ib = -1; int jfound = -1;
    for(int j=0;j<nb;j++){
      if(strcmp(name,b[j].name)==0){ ib = parse_inode(b[j].link); jfound=j; break; }
    }
    printf("  %-6s: %s  vs  ", name, a[i].link);
    if(ib==-1) printf("(não encontrado)\n");
    else {
      printf("%s", b[jfound].link);
      if(ia!=ib){ printf("  <-- DIFERENTE"); diff=1; }
      printf("\n");
    }
  }
  return diff;
}

/* ---------- map (group by inode) ---------- */
typedef struct { long long inode; int pid; } pair_t;
static int cmp_pair(const void* a, const void* b){
  const pair_t* x=(const pair_t*)a; const pair_t* y=(const pair_t*)b;
  if(x->inode < y->inode) return -1;
  if(x->inode > y->inode) return 1;
  if(x->pid   < y->pid)   return -1;
  if(x->pid   > y->pid)   return 1;
  return 0;
}

bool ns_map_processes_by_type(const char* type){
  if(!has_type(type)){ fprintf(stderr,"tipo inválido: %s\n", type); return false; }
  pair_t* vec = NULL; size_t cap=0, sz=0;

  DIR* d = opendir("/proc"); if(!d){ perror("opendir /proc"); return false; }
  struct dirent* e;
  while( (e=readdir(d)) ){
    if(e->d_type!=DT_DIR) continue;
    int pid = atoi(e->d_name);
    if(pid<=0) continue;

    char path[64]; snprintf(path,sizeof(path),"/proc/%d/ns/%s", pid, type);
    char link[128]; ssize_t r = readlink(path, link, sizeof(link)-1);
    if(r<0) continue;
    link[r]=0;

    long long ino = parse_inode(link); if(ino<0) continue;

    if(sz==cap){ cap = cap? cap*2: 1024; vec = realloc(vec, cap*sizeof(pair_t)); }
    vec[sz++] = (pair_t){.inode=ino,.pid=pid};
  }
  closedir(d);

  if(sz==0){ printf("nenhum processo encontrado para %s\n", type); free(vec); return true; }

  qsort(vec, sz, sizeof(pair_t), cmp_pair);
  printf("# grupos por %s-namespace (inode -> pids)\n", type);
  long long cur=-1;
  for(size_t i=0;i<sz;i++){
    if(vec[i].inode!=cur){
      cur = vec[i].inode;
      printf("%s:[%lld]: ", type, cur);
    }
    printf("%d", vec[i].pid);
    if(i+1==sz || vec[i+1].inode!=cur) printf("\n"); else printf(" ");
  }
  free(vec);
  return true;
}

/* ---------- overhead (unshare) ---------- */
static int flags_from_csv(const char* csv){
  int f=0;
  char buf[128]; strncpy(buf,csv,sizeof(buf)); buf[sizeof(buf)-1]=0;
  for(char* tok=strtok(buf,","); tok; tok=strtok(NULL,",")){
    if(strcmp(tok,"user")==0)      f|=CLONE_NEWUSER;
    else if(strcmp(tok,"mnt")==0)  f|=CLONE_NEWNS;
    else if(strcmp(tok,"uts")==0)  f|=CLONE_NEWUTS;
    else if(strcmp(tok,"ipc")==0)  f|=CLONE_NEWIPC;
    else if(strcmp(tok,"pid")==0)  f|=CLONE_NEWPID;
    else if(strcmp(tok,"net")==0)  f|=CLONE_NEWNET;
    else if(strcmp(tok,"cgroup")==0) f|=CLONE_NEWCGROUP;
  }
  return f;
}

bool ns_measure_overhead(const char* flags_csv, int runs){
  int flags = flags_from_csv(flags_csv);
  if(!flags){ fprintf(stderr,"flags vazias/invalidas\n"); return false; }
  long long total=0; int ok=0, fail=0;

  for(int i=0;i<runs;i++){
    int fd[2]; if(pipe(fd)!=0){ perror("pipe"); return false; }
    pid_t p = fork();
    if(p==0){
      close(fd[0]);
      long long t0=now_us();
      int rc = unshare(flags);
      long long t1=now_us();
      long long delta = (rc==0)? (t1-t0) : - (t1-t0);
      (void)!write(fd[1], &delta, sizeof(delta));
      close(fd[1]);
      _exit(rc==0?0:1);
    }else if(p>0){
      close(fd[1]);
      long long delta=0; (void)!read(fd[0], &delta, sizeof(delta));
      close(fd[0]);
      int status=0; waitpid(p,&status,0);
      if(delta>0 && WIFEXITED(status) && WEXITSTATUS(status)==0){ total+=delta; ok++; }
      else { fail++; }
    }else{
      perror("fork"); return false;
    }
  }

  printf("overhead unshare(%s): runs=%d ok=%d fail=%d", flags_csv, runs, ok, fail);
  if(ok>0) printf("  avg=%.1f us\n", (double)total/(double)ok);
  else printf("\n");
  if(fail>0) printf("(obs: falhas podem ser por EPERM; em WSL, CLONE_NEWNET costuma falhar)\n");
  return true;
}

/* ---------- CLI ---------- */
static void usage_cli(void){
  fprintf(stderr,
    "uso: ns_analyzer <comando> [args]\n"
    "comandos:\n"
    "  list --pid <PID>\n"
    "  compare --pid <A> --pid2 <B>\n"
    "  map --type <mnt|uts|ipc|pid|user|net|cgroup>\n"
    "  overhead --flags <csv> [--runs N]\n");
}

int main(int argc, char** argv){
  if(argc<2){ usage_cli(); return 1; }
  if(strcmp(argv[1],"list")==0){
    int pid=-1; for(int i=2;i<argc;i++) if(!strcmp(argv[i],"--pid")&&i+1<argc) pid=atoi(argv[++i]);
    if(pid<=0){ usage_cli(); return 1; }
    ns_entry_t e[8]; int n=ns_list_for_pid(pid,e,8);
    if(n<=0){ fprintf(stderr,"não consegui ler /proc/%d/ns\n", pid); return 2; }
    printf("namespaces do PID %d:\n", pid);
    for(int i=0;i<n;i++) printf("  %-6s -> %s\n", e[i].name, e[i].link);
    return 0;
  } else if(strcmp(argv[1],"compare")==0){
    int a=-1,b=-1;
    for(int i=2;i<argc;i++){
      if(!strcmp(argv[i],"--pid")&&i+1<argc) a=atoi(argv[++i]);
      else if(!strcmp(argv[i],"--pid2")&&i+1<argc) b=atoi(argv[++i]);
    }
    if(a<=0||b<=0){ usage_cli(); return 1; }
    return ns_compare_pids(a,b)!=0;
  } else if(strcmp(argv[1],"map")==0){
    const char* t=NULL; for(int i=2;i<argc;i++) if(!strcmp(argv[i],"--type")&&i+1<argc) t=argv[++i];
    if(!t){ usage_cli(); return 1; }
    return ns_map_processes_by_type(t)?0:2;
  } else if(strcmp(argv[1],"overhead")==0){
    const char* flags="user"; int runs=20;
    for(int i=2;i<argc;i++){
      if(!strcmp(argv[i],"--flags")&&i+1<argc) flags=argv[++i];
      else if(!strcmp(argv[i],"--runs")&&i+1<argc) runs=atoi(argv[++i]);
    }
    return ns_measure_overhead(flags,runs)?0:2;
  } else {
    usage_cli(); return 1;
  }
}
