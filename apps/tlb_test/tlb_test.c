#include <klib.h>
#include "tlb_test.h"

void main(){
    unsigned long long instrs,cycles;
    printf("start Virtual Memory initialize with custom segments\n");
    _vme_init_custom(sv39_pgalloc, sv39_pgfree, segments, sizeof(segments) / sizeof(segments[0]));
    printf("finish\n");
    tlb_hit_test(&instrs,&cycles);
    printf("instrs %d cycles %d\n",instrs,cycles);
    _halt(0);


}


