#include <math.h>
int main(){ volatile double x=0; for(long long i=0;i<1000000000LL;i++) x+=sin(i); return (int)x; }
