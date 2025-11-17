#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(){ FILE* f=fopen("/tmp/io_test.bin","wb"); if(!f) return 1;
  const size_t B=1024*1024; char* buf=malloc(B);
  for(int i=0;i<256;i++){ for(size_t j=0;j<B;j++) buf[j]=(char)(i+j);
    fwrite(buf,1,B,f); fflush(f); usleep(50000);}
  fclose(f); free(buf); return 0; }
