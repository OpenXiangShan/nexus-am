#include "common.h"

static volatile long long test_data[1024] __attribute__((aligned(4096))); // 使用64位整数类型

void __attribute__ ((noinline)) loadlat(int cnt) {
#define ONE "ld t0, 0(t0)\n\t"    // 改用ld指令加载64位数据

    // 初始化测试数据
    for(int i = 0; i < 1023; i++) {
        test_data[i] = (long long)(&test_data[i+1]); // 使用64位整数存储地址
    }
    test_data[1023] = (long long)(&test_data[0]); // 最后一个元素指向开头，形成环

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
  loadlat(1000);
  return 0;
}