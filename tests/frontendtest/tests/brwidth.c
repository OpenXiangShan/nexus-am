
#include "common.h"

void __attribute__ ((noinline)) brwidth(int cnt){
#define ONE "beqz t0, 1b\n\t"    // 由于t0始终非零，分支永远不会taken

    asm volatile(
        "li t0, 1\n\t"           // 初始化t0为1，确保分支不会taken
        ".align 4\n\t"
        "1:\n\t"
        "addi t0, t0, 1\n\t"     // 确保t0始终为非零
        HUNDRED                   // 100个分支指令
        "addi %0, %0, -1\n\t"
        "bnez %0, 1b\n\t"
        : 
        :"r"(cnt)
        :"t0", "memory");
}



int main() {
  brwidth(100);
  return 0;
}
