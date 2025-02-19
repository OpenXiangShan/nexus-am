#include "common.h"

void __attribute__ ((noinline)) branch_test(int cnt) {
    int tmp = 0;
    // 设计交替的分支模式：
    // 通过判断t2的奇偶性来实现T-NT交替
    #define ONE \
        "andi t4, t2, 1\n\t"     /* 检查t2是否为奇数 */ \
        "beqz t4, 2f\n\t"        /* 如果是偶数则跳转 */ \
        "nop\n\t" \
        "2:\n\t"

    asm volatile(
        // 初始化计数器
        "li t0, 0\n\t"      // 循环计数器
        "li t2, 0\n\t"      // 用于产生交替模式的计数器
        ".align 4\n\t"
        "1:\n\t"
        // 重复执行分支指令
        TEN                  // 每次循环执行10次分支指令
        "addi t2, t2, 1\n\t" // t2每次加1，产生0,1,2,3...序列
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

// 这种情况下，base 表几乎都完全命中，导致几乎不会分配新的tage 表项