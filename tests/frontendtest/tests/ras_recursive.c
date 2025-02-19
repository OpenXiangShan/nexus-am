#include "common.h"
// #include <stdio.h>

// 使用 volatile 防止编译器优化
volatile int global_sum = 0;

// 递归函数，确保每次调用都会改变全局状态
__attribute__((noinline)) // 防止内联优化
int recursive_test(int depth) {
    // 改变全局状态，防止编译器优化
    global_sum += depth;
    
    if (depth <= 0) {
        return global_sum;
    }
    
    // 做一些计算，确保编译器不会优化掉这个函数
    int result = recursive_test(depth - 1) + 1;
    global_sum += result;
    return result;
}


int main() {
    recursive_test(12);  // 从深度5开始测试
    return 0;
}