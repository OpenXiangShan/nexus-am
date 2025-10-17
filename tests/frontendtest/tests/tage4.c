
int main() {
    // 绑定i 到寄存器
    // 方法1: 使用register关键字提示编译器将i分配到寄存器
    register unsigned long i;
    for (i = 0; i < 1000; i++) {
        if (i % 2 == 0) {
            asm volatile("nop");
        }
    }
    return 0;
}