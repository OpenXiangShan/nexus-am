#include "common.h"

void __attribute__ ((noinline)) branch_test(int cnt) {
    int sum = 0;
    asm volatile(
        "li t0, 0\n\t"          // 循环计数器初始化为0
        "1:\n\t"
        "addi %0, %0, 1\n\t"    // sum++
        "addi t0, t0, 1\n\t"    // i++
        "blt t0, %1, 1b\n\t"    // if (i < cnt) goto loop
        : "+r"(sum)
        : "r"(cnt)
        : "t0"
    );
}

int main() {
    branch_test(100);  // 执行10000次循环
    return 0;
}