#include <klib.h>
#include "cachetest_o.h"

//#define test_log CHOOSE
#define xstr(s) str(s)
#define str(s)  #s


int main(){
   // printf("111111\n");
   unsigned long long busy_cycles_l1,busy_cycles_l2,busy_cycles_l3;
   unsigned long long busy_instrs_l1,busy_instrs_l2,busy_instrs_l3;
    
 //   unsigned long long busy_cycles_l1,busy_cycles_l2;
   // unsigned long long busy_instrs_l1,busy_instrs_l2;
//    unsigned long long busy_cycles_l1;
//    unsigned long long busy_instrs_l1;

    //uint64_t end_addr_l1 = (_PERF_TEST_ADDR_BASE + _PERF_L1_SIZE_BYTE/2 + _TEST_NUM_SIZE );    
    //uint64_t end_addr_l1 = (_PERF_TEST_ADDR_BASE + _PERF_L1_SIZE_BYTE/2 + _TEST_NUM_SIZE );   
    uint64_t end_addr_l1 = (_PERF_TEST_ADDR_BASE + _PERF_L1_SIZE_BYTE - (2*_TEST_NUM_SIZE)); 
    uint64_t end_addr_l2 = (_PERF_TEST_ADDR_BASE  + _PERF_L1_SIZE_BYTE + (_PERF_L2_SIZE_BYTE/2) + (2*_TEST_NUM_SIZE));
    uint64_t end_addr_l3 = (_PERF_TEST_ADDR_BASE + _PERF_L1_SIZE_BYTE + _PERF_L2_SIZE_BYTE + _PERF_L2_SIZE_BYTE );

    uint64_t start_addr = _PERF_TEST_ADDR_BASE;
    printf("start_addr %x,end_addr %x\n",start_addr,end_addr_l1);

    full_cache_init(start_addr,end_addr_l1,_PERF_CACHELINE_SIZE_BYTE,1);
    cache_loop_l1d(&busy_instrs_l1,&busy_cycles_l1,atoi(xstr(INST)));

    full_cache_init(start_addr,end_addr_l2,_PERF_CACHELINE_SIZE_BYTE,1);
    cache_loop(&busy_instrs_l2,&busy_cycles_l2,atoi(xstr(INST)));
    full_cache_init(start_addr,end_addr_l3,_PERF_CACHELINE_SIZE_BYTE,1);
    cache_loop(&busy_instrs_l3,&busy_cycles_l3,atoi(xstr(INST)));

    printf("the num of load is %s\n",xstr(CHOOSE));
    printf("the test of load|...|load| hit in l1\n");
    printf("instrs %d l1-cycles %d\n",busy_instrs_l1,busy_cycles_l1);
    printf("the test of load|...|load 12 hit in l2\n");
    printf("instrs %d l2-cycles %d\n",busy_instrs_l2,busy_cycles_l2);
    printf("the test of load|...|load hit in l3\n");
    printf("instrs %d l3-cycles %d\n",busy_instrs_l3,busy_cycles_l3);



}

