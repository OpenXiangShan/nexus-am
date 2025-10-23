
#include "common.h"

void __attribute__ ((noinline)) aluwidth(int cnt){
#define ONE \
    "add t0, t0, a0\n\t"  \
    "add t1, t1, a0\n\t"  \
    "add t2, t2, a0\n\t"  \
    "add t3, t3, a0\n\t"  \
    "add t4, t4, a0\n\t"  \
    "add t5, t5, a0\n\t"  \
    "add t6, t6, a0\n\t"  \
    "add s2, s2, a0\n\t"

    asm volatile(
        // 初始化所有目标寄存器
        "li t0, 1\n\t"
        "li t1, 1\n\t"
        "li t2, 1\n\t"
        "li t3, 1\n\t"
        "li t4, 1\n\t"
        "li t5, 1\n\t"
        "li t6, 1\n\t"
        "li s2, 1\n\t"
        "li a0, 1\n\t"      // 源操作数
        ".align 4\n\t"
        "1:\n\t"
        HUNDRED
        "addi %0, %0, -1\n\t"
        "bnez %0, 1b\n\t"
        : 
        :"r"(cnt)
        :"t0", "t1", "t2", "t3", "t4", "t5", "t6", 
         "s2", "a0", "memory");
}



int main() {
  aluwidth(200);
  return 0;
}
