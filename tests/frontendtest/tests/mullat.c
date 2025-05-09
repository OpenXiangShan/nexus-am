#include "common.h"

static volatile long test_data[1024] __attribute__((aligned(4096))); // 声明一个对齐的数组用于测试

void __attribute__ ((noinline)) loadlat(int cnt) {
#define ONE "lw t0, 0(t0)\n\t"    // t0 = MEM[t0]，每次load都依赖上一次的结果

    // 初始化测试数据
    for(int i = 0; i < 1023; i++) {
        test_data[i] = (long)(&test_data[i+1]); // 每个元素存储下一个元素的地址
    }
    test_data[1023] = (long)(&test_data[0]); // 最后一个元素指向开头，形成环

    asm volatile(
        "la t0, test_data\n\t"     // 初始化t0为测试数组的地址
        ".align 4\n\t"
        "1:\n\t"
        THOUSAND                    // 100个连续依赖的load指令
        "addi %0, %0, -1\n\t"
        "bnez %0, 1b\n\t"
        : 
        :"r"(cnt)
        :"t0", "memory");
}

int main() {
  loadlat(__LOOP_COUNT * 10);
  return 0;
}