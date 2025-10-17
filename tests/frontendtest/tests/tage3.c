#include "common.h"

void __attribute__ ((noinline)) branch_test(int cnt) {
    int tmp = 0;
    // 设计一个需要更长历史的分支模式：
    // 使用t2的多个位来决定分支方向
    // 比如：当t2的bit[0]为1跳转，为0 NT
    // 修改后的分支模式：
    #define ONE \
        "andi t3, t2, 1\n\t"    /* 取低1位 */ \
        "li t4, 1\n\t"          /* 目标值1 */ \
        "bne t3, t4, 2f\n\t"     /* 如果不等于1则跳转 */ \
        "nop\n\t" \
        "2:\n\t"

    asm volatile(
        // 初始化计数器
        "li t0, 0\n\t"      // 循环计数器
        "li t2, 0\n\t"      // 模式计数器
        ".align 4\n\t"
        "1:\n\t"
        // 重复执行分支指令
        ONE                  
        "addi t2, t2, 1\n\t" // t2每次加1
        "addi t0, t0, 1\n\t" // 增加循环计数器
        "blt t0, %1, 1b\n\t" // 循环控制
        : "+r"(tmp)
        : "r"(cnt)
        : "t0", "t2", "t3", "t4", "memory"
    );
}

int main() {
    branch_test(1000);  // 执行1000次循环
    return 0;
}
/*
t2值    3位pattern    跳转
0       000           T
1       001           T
2       010           T
3       011           T
4       100           T
5       101           T
6       110           T
7       111           NT
8       100           T
9       101           T
10      110           T
11      111           T
12      100           T

应该8个值有一个not taken

1000次循环，1000/8=125， 跳转太少了, 当然每个循环内部都由10个branch
所以如果tage不准的话，应该有1250次NT, 其他都是taken
...*/