
#include "common.h"

void __attribute__ ((noinline)) renamewidth(int cnt){
#define ONE \
    "fadd.d f2, f0, f1\n\t"     \
    "add t2, a0, zero\n\t"      \
    "add t3, a0, zero\n\t"      \
    "fadd.d f3, f0, f1\n\t"     \
    "add t4, a0, zero\n\t"      \
    "add t5, a0, zero\n\t"      \
    "fadd.d f4, f0, f1\n\t"     \
    "add t6, a0, zero\n\t"      \
    "add s2, a0, zero\n\t"      

    asm volatile(
        // 初始化寄存器
        "fmv.d.x f0, zero\n\t"   // 源浮点寄存器
        "li t0, 1\n\t"
        "fmv.d.x f1, t0\n\t"     // 源浮点寄存器
        "li a0, 123\n\t"         // 源整数寄存器
        ".align 4\n\t"
        "1:\n\t"
        HUNDRED
        "addi %0, %0, -1\n\t"
        "bnez %0, 1b\n\t"
        : 
        :"r"(cnt)
        :"t0", "t2", "t3", "t4", "t5", "t6", "s2", "a0",
         "f0", "f1", "f2", "f3", "f4", "memory");
}



int main() {
  renamewidth(__LOOP_COUNT);
  return 0;
}
