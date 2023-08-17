#include <klib.h>
#include "stream_pre_test.h"

int main(){
    unsigned long long cycles,instrs;
    unsigned long long cycles0,instrs0;
    unsigned long long cycles1,instrs1;
    stream_pre(&instrs,&cycles);
    instrs0 = csr_read(CSR_MINSTRET);
    cycles0 = csr_read(CSR_MCYCLE);
    stream_pre_address(72,320,_PERF_TEST_ADDR_BASE_1+_LAST_ADDR_BYTE);
    instrs1 = csr_read(CSR_MINSTRET);
    cycles1 = csr_read(CSR_MCYCLE);
    printf("the test of stream_pre \n");
    printf("instrs %d cycles %d\n",instrs,cycles);
    printf("instrs1 %d cycles1 %d\n",instrs1-instrs0,cycles1-cycles0);
}