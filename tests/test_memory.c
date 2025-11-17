#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(){ const size_t step=10*1024*1024; for(;;){ void* p=malloc(step); if(!p) break; memset(p,0,step); usleep(200000);} }
