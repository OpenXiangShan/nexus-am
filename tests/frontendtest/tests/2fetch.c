#include "common.h"

void __attribute__ ((noinline)) test_2fetch(int cnt) {
    // 创建两个3条指令的基本块，在同一cache行内相互跳转
    // 基本块A: 3条指令（2条算术 + 1条跳转）
    // 中间填充2条nop指令，保证两个基本块不连续
    // 基本块B: 3条指令（2条算术 + 1条跳转回A）
    
    asm volatile(
        // 初始化寄存器
        "li t1, 1\n\t"
        "li t2, 2\n\t"
        "li t3, 3\n\t"
        
        // 确保32字节对齐，保证两个基本块在同一cache行
        ".align 5\n\t"  // 32字节对齐
        
        "loop_start:\n\t"
        
        // 基本块A (3条指令)
        "block_a:\n\t"
        "add t4, t1, t2\n\t"       // 指令1: 算术运算
        "add t5, t2, t3\n\t"       // 指令2: 算术运算  
        "j block_b\n\t"            // 指令3: 跳转到基本块B

        "nop\n\t"                  // 填充
        "nop\n\t"                  // 填充
        
        // 基本块B (3条指令) - 紧接着基本块A，在同一cache行内
        "block_b:\n\t"
        "add t6, t4, t5\n\t"       // 指令1: 算术运算
        "addi %0, %0, -1\n\t"      // 指令2: 循环计数器减1
        "bnez %0, block_a\n\t"     // 指令3: 如果计数器不为0，跳回基本块A
        
        : "+r"(cnt)  // 输入输出：cnt作为循环计数器
        :            // 无纯输入
        : "t1", "t2", "t3", "t4", "t5", "t6", "memory"  // 破坏的寄存器
    );
}

int main() {
    // 执行测试，循环1000次
    // 如果CPU支持2fetch：两个基本块可以在一拍内取指
    // 如果CPU不支持2fetch：需要两拍分别取指
    test_2fetch(10000);
    
    return 0;
}
