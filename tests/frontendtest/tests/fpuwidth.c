
#include "common.h"

void __attribute__ ((noinline)) fpuwidth(int cnt){
#define ONE \
    "fadd.d f2,  f0, f1\n\t" \
    "fadd.d f3,  f0, f1\n\t" \
    "fadd.d f4,  f0, f1\n\t" \
    "fadd.d f5,  f0, f1\n\t" \
    "fadd.d f6,  f0, f1\n\t" \
    "fadd.d f7,  f0, f1\n\t" \
    "fadd.d f8,  f0, f1\n\t" \
    "fadd.d f9,  f0, f1\n\t"

    asm volatile(
        // 初始化浮点寄存器
        "fmv.d.x f0, zero\n\t"  // 设置 f0 为 0.0
        "li t0, 1\n\t"
        "fmv.d.x f1, t0\n\t"    // 设置 f1 为 1.0
        ".align 4\n\t"
        "1:\n\t"
        HUNDRED
        "addi %0, %0, -1\n\t"
        "bnez %0, 1b\n\t"
        : 
        :"r"(cnt)
        :"t0", "f0", "f1", "f2", "f3", "f4", "f5", 
         "f6", "f7", "f8", "f9", "memory");
}


int main() {
  fpuwidth(100);
  return 0;
}
