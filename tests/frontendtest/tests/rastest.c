#include "common.h"

void __attribute__ ((noinline)) ras_test(int cnt) {
    asm volatile(
        // 保存返回地址
        "addi sp, sp, -16\n\t"
        "sd ra, 8(sp)\n\t"
        
        // 初始化计数器
        "li a5, 0\n\t"          // sum = 0
        "li t0, 0\n\t"          // i = 0
        
        // 主循环
        "1:\n\t"
        "addi a5, a5, 1\n\t"    // sum++
        "addi t0, t0, 1\n\t"    // i++
        "jal ra, 2f\n\t"        // 调用函数
        "blt t0, %0, 1b\n\t"    // if (i < cnt) goto loop
        "j 3f\n\t"              // 跳到结束
        
        // 简单函数
        "2:\n\t"
        "addi a5, a5, 2\n\t"    // sum += 2
        "ret\n\t"               // 返回
        
        // 结束
        "3:\n\t"
        "ld ra, 8(sp)\n\t"
        "addi sp, sp, 16\n\t"
        :
        : "r"(cnt)
        : "a5", "t0", "ra", "memory"
    );
}

int main() {
    ras_test(100);  // 执行10次简单的call-ret对
    return 0;
}