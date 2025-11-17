#define _GNU_SOURCE
#include "monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// soma a primeira linha "cpu ..." de /proc/stat
static int read_total_jiffies(uint64_t* total){
    FILE* f = fopen("/proc/stat", "r");
    if(!f) return 0;
    char tag[8]; // "cpu"
    unsigned long long v[10]={0};
    int n = fscanf(f, "%7s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                   tag,&v[0],&v[1],&v[2],&v[3],&v[4],&v[5],&v[6],&v[7],&v[8],&v[9]);
    fclose(f);
    if(n < 5) return 0;
    unsigned long long sum=0;
    for(int i=0;i<10;i++) sum += v[i];
    *total = sum;
    return 1;
}

// pega utime e stime do /proc/<pid>/stat
static int read_proc_jiffies(int pid, uint64_t* proc){
    char path[64]; snprintf(path,sizeof(path),"/proc/%d/stat",pid);
    FILE* f = fopen(path,"r"); if(!f) return 0;

    // lê uma linha inteira
    char* line = NULL; size_t cap = 0;
    ssize_t len = getline(&line,&cap,f);
    fclose(f);
    if(len<=0){ free(line); return 0; }

    // encontra o ')' que fecha o comm
    char* rparen = strrchr(line, ')');
    if(!rparen){ free(line); return 0; }
    // a partir de depois do ')', começam os campos do 3 em diante (state é o 3)
    char* rest = rparen + 2; // pula ") "

    // tokeniza
    int idx = 1; // campo 3 => idx 1
    char* save = NULL;
    char* tok = strtok_r(rest, " ", &save);
    unsigned long long ut=0, st=0;
    while(tok){
        if(idx==12) ut = strtoull(tok,NULL,10);   // campo 14
        if(idx==13) { st = strtoull(tok,NULL,10); break; } // campo 15
        tok = strtok_r(NULL," ",&save);
        idx++;
    }
    free(line);
    if(ut==0 && st==0) return 0;
    *proc = ut + st;
    return 1;
}

bool read_cpu_times(int pid, cpu_times_t* out){
    uint64_t p=0,t=0;
    if(!read_proc_jiffies(pid,&p)) return false;
    if(!read_total_jiffies(&t)) return false;
    out->proc_jiffies = p;
    out->total_jiffies = t;
    return true;
}

double cpu_percent(const cpu_times_t* prev, const cpu_times_t* now){
    if(!prev || !now) return 0.0;
    const long ncpu = sysconf(_SC_NPROCESSORS_ONLN) > 0 ? sysconf(_SC_NPROCESSORS_ONLN) : 1;
    const double dproc = (double)(now->proc_jiffies - prev->proc_jiffies);
    const double dtot  = (double)(now->total_jiffies - prev->total_jiffies);
    if(dproc < 0 || dtot <= 0) return 0.0;
    // percent em relação a 1 CPU (normaliza pelo total agregado e multiplica por ncpu)
    return (dproc / dtot) * 100.0 * (double)ncpu;
}
