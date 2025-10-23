
#include "common.h"

void __attribute__ ((noinline)) ifuwidth(int cnt) {
    // ... existing code ...
    
    #define ONE \
        "add t1, t2, t3\n\t"     /* 整数加法 1 */ \
        "add t4, t2, t3\n\t"     /* 整数加法 2 */ \
        "add t5, t2, t3\n\t"     /* 整数加法 3 */ \
        "add t6, t2, t3\n\t"     /* 整数加法 4 */ \
        "fadd.d f2, f0, f1\n\t"  /* 浮点加法 1 */ \
        "fadd.d f3, f0, f1\n\t"  /* 浮点加法 2 */ \
        "fadd.d f4, f0, f1\n\t"  /* 浮点加法 3 */ \
        "fadd.d f5, f0, f1\n\t"  /* 浮点加法 4 */

    asm volatile(
        // 初始化整数寄存器
        "li t2, 1\n\t"
        "li t3, 2\n\t"
        // 初始化浮点寄存器
        "fmv.d.x f0, t2\n\t"
        "fmv.d.x f1, t3\n\t"
        ".align 4\n\t"
        "1:\n\t"
        HUNDRED
        "addi %0, %0, -1\n\t"
        "bnez %0, 1b\n\t"
        : 
        :"r"(cnt)
        :"t1", "t2", "t3", "t4", "t5", "t6",
         "f0", "f1", "f2", "f3", "f4", "f5", "memory");
}


int main() {
  ifuwidth(100);
//   perf_counter_t perf;
//   measure_performance(ifuwidth, 1000, &perf);
//   printf("\nIFU Test Performance:\n");
//   printf("Cycles: %lu\n", perf.cycles);
//   printf("Instructions: %lu\n", perf.instructions);
//   printf("IPC: %.2f\n", perf.ipc);
    // printf 会大幅降低最后的ipc!!!
  return 0;
}
