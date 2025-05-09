#include "common.h"
#include <klib.h>



// 测试多个连续的条件分支跳转
void __attribute__ ((noinline)) branch_test(int cnt) {
    int tmp = 0;

#define ONE "beqz t1, 2f\n\t" \
    "addi %0, %0, 1\n\t" \
    "2:\n\t"

    asm volatile(
        // 初始化计数器
        "li t1, 0\n\t"      // 循环计数器
        ".align 4\n\t"
        "1:\n\t"
        // 创建连续的条件分支跳转模式
        HUNDRED
        "addi t0, t0, 1\n\t"    // 增加循环计数器
        "blt t0, %1, 1b\n\t"    // 循环控制
        : "+r"(tmp)
        : "r"(cnt)
        : "t0", "memory"
    );
}
#undef ONE

// 测试多个连续的条件分支跳转
void __attribute__ ((noinline)) branch_test1(int cnt) {
    int tmp = 0;
// 通过取余数实现每10次跳转1次
// 一次跳转，一次不跳转
#define ONE \
    "addi t2, t2, 1\n\t" \
    "andi t1, t2, 1\n\t" \
    "beqz t1, 2f\n\t" \
    "2:\n\t" \
    "beqz t1, 3f\n\t" \
    "nop\n\t" \
    "3:\n\t" \
    "j 4f\n\t" \
    "nop\n\t" \
    "4:\n\t"

    asm volatile(
        // 初始化计数器
        "li t0, 0\n\t"      // 循环计数器
        "li t2, 0\n\t"
        "li t3, 10\n\t"
        ".align 4\n\t"
        "1:\n\t"
        // 创建连续的条件分支跳转模式
        HUNDRED
        "addi t0, t0, 1\n\t"    // 增加循环计数器
        "blt t0, %1, 1b\n\t"    // 循环控制
        : "+r"(tmp)
        : "r"(cnt)
        : "t0", "t1", "t2", "memory"
    );
}
#undef ONE

// 测试多个连续的条件分支跳转
void __attribute__ ((noinline)) branch_test2(int cnt) {
    int tmp = 0;
// 前3次循环会跳转，之后都不跳转
#define ONE \
    "addi t2, t2, 1\n\t" \
    "blt t2, t4, 2f\n\t" \
    "nop\n\t" \
    "2:\n\t" \
    "blt t2, t4, 3f\n\t" \
    "nop\n\t" \
    "3:\n\t" \
    "j 4f\n\t" \
    "nop\n\t" \
    "4:\n\t"

    asm volatile(
        // 初始化计数器
        "li t0, 0\n\t"      // 循环计数器
        "li t2, 0\n\t"
        "li t4, 30\n\t"      // 前30次循环跳转阈值, 30=10*3，因为每个ONE被展开为10次
        ".align 4\n\t"
        "1:\n\t"
        // 创建连续的条件分支跳转模式
        TEN
        "addi t0, t0, 1\n\t"    // 增加循环计数器
        "blt t0, %1, 1b\n\t"    // 循环控制
        : "+r"(tmp)
        : "r"(cnt)
        : "t0", "t1", "t2", "t4", "memory"
    );
}

int main() {

    branch_test2(100);  // 执行足够多的循环来训练分支预测器
    return 0;
}