#include "common.h"
#include <klib.h>

// 用于填充内容，让FB 超过32Byte
#define dummy_inst "lui  t6,0x40600\n\t"

// 测试numBr 不同时候的测试
// numBr = 2 , 一个基本块放不下3个分支
// numBr = 4, 能放下3个分支， commitFsqEntryHasInsts=5
// 如果fetchBlock = 64Byte, 那么能放下更多的分支
void __attribute__ ((noinline)) branch_test2(int cnt) {
    int tmp = 0;
// 前3次循环会跳转，之后都不跳转
// 第一个分支跳转阈值为3，第二个分支跳转阈值为5， 先taken, 后not taken
// 第三个，第四个分支和第二个分支相同，第五个为jump
#define ONE \
    "blt t2, t4, 2f\n\t" \
    "nop\n\t" \
    "2:\n\t" \
    dummy_inst \
    dummy_inst \
    "blt t3, t5, 3f\n\t" \
    "nop\n\t" \
    "3:\n\t" \
    dummy_inst \
    dummy_inst \
    "blt t3, t5, 4f\n\t" \
    "nop\n\t" \
    "4:\n\t" \
    dummy_inst \
    dummy_inst \
    "blt t3, t5, 5f\n\t" \
    "nop\n\t" \
    "5:\n\t" \
    dummy_inst \
    dummy_inst \
    "j 6f\n\t" \
    "nop\n\t" \
    "6:\n\t"

    asm volatile(
        // 初始化计数器
        "li t0, 0\n\t"      // 循环计数器
        "li t2, 0\n\t"
        "li t4, 3\n\t"      // 第一个分支跳转阈值
        "li t5, 5\n\t"     // 第二个分支跳转阈值
        ".align 4\n\t"
        "1:\n\t"
        // 创建连续的条件分支跳转模式
        TEN
        "addi t2, t2, 1\n\t"
        "addi t3, t3, 1\n\t"    // 每次大循环，t3 自增1
        "addi t0, t0, 1\n\t"    // 增加循环计数器
        "blt t0, %1, 1b\n\t"    // 循环控制
        : "+r"(tmp)
        : "r"(cnt)
        : "t0", "t1", "t2", "t3", "t4", "t5", "memory"
    );
}

int main() {

    branch_test2(100);  // 执行足够多的循环来训练分支预测器
    return 0;
}