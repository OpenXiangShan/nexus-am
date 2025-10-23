
#include "common.h"

void __attribute__ ((noinline)) fetchfrag(int cnt) {
    // ... existing code ...
    
    // // 基本块长度为4
    /* 方式1：使用块注释
    #define ONE \
        "add t1, t2, t3\n\t"     // 整数加法 1  \
        "add t4, t2, t3\n\t"     // 整数加法 2  \
        "add t5, t2, t3\n\t"     // 整数加法 3  \
        "j 2f\n\t" \
        "2:\n\t"
    */

        // 基本块长度为4, add+j+add+j
        /*
    #define ONE \
        "add t1, t2, t3\n\t" \
        "j 2f\n\t" \
        "add t4, t2, t3\n\t" \
        "2:\n\t" \
        "add t4, t2, t3\n\t" \
        "j 3f\n\t" \
        "add t5, t2, t3\n\t" \
        "3:\n\t"
    */

    // 基本块长度为4, add+beq+add+beq
    // 4个br, 前三个NT, 最后一个T,并且32byte放得下
    #define ONE \
        "c.add t1, t2\n\t" \
        "bnez t0, 2f\n\t" \
        "c.add t4, t2\n\t" \
        "2:\n\t" \
        "c.add t4, t2\n\t" \
        "bnez t0, 3f\n\t" \
        "c.add t5, t2\n\t" \
        "3:\n\t" \
        "c.add t4, t2\n\t" \
        "bnez t0, 4f\n\t" \
        "c.add t5, t2\n\t" \
        "4:\n\t" \
        "c.add t4, t2\n\t" \
        "beqz t0, 5f\n\t" \
        "c.add t5, t2\n\t" \
        "5:\n\t"
    

    asm volatile(
        // 初始化整数寄存器
        "li t2, 1\n\t"
        "li t3, 2\n\t"
        ".align 4\n\t"
        "1:\n\t"
        // TEN
        HUNDRED
        "addi %0, %0, -1\n\t"
        "bnez %0, 1b\n\t"
        : 
        :"r"(cnt)
        :"t1", "t2", "t3", "t4", "t5", "t6",
         "f0", "f1", "f2", "f3", "f4", "f5", "memory");
}


int main() {
  fetchfrag(100);
  return 0;
}
