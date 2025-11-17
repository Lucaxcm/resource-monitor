#include "monitor.h"
#include <stdio.h>
#include <string.h>

bool read_net_totals_all(net_totals_t* out){
    FILE* f = fopen("/proc/net/dev","r");
    if(!f) return false;

    char line[512];
    if(!fgets(line,sizeof(line),f)) { fclose(f); return false; }
    if(!fgets(line,sizeof(line),f)) { fclose(f); return false; }

    unsigned long long rx=0, tx=0;
    while (fgets(line,sizeof(line),f)){
        char ifname[64];
        unsigned long long rbytes, rpkts, rerrs, rdrop, rfifo, rframe, rcomp, rmult;
        unsigned long long tbytes, tpkts, terrs, tdrop, tfifo, tcolls, tcar, tcomp;

        if (sscanf(line," %63[^:]: %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                   ifname,&rbytes,&rpkts,&rerrs,&rdrop,&rfifo,&rframe,&rcomp,&rmult,
                   &tbytes,&tpkts,&terrs,&tdrop,&tfifo,&tcolls,&tcar,&tcomp) == 17){
            rx += rbytes;
            tx += tbytes;
        }
    }
    fclose(f);
    out->rx_bytes = rx;
    out->tx_bytes = tx;
    return true;
}
