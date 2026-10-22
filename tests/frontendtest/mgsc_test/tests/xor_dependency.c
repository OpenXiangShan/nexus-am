#include <klib.h>
static volatile int sink;

int main(void) {
    int last_b0 = 0, last_b1 = 0;
    for (int i = 0; i < 6000; i++) {
        // B0：周期 3 模式
        int b0 = (i % 3) ? 1 : 0;
        if (b0) sink++;

        // B1：周期 5 模式
        int b1 = (i % 5) ? 0 : 1;
        if (b1) sink += 2;

        // B2：依赖上一轮 B0^B1
        int pred = last_b0 ^ last_b1;
        if (pred) {
            sink += 3;
        } else {
            sink -= 3;
        }

        last_b0 = b0;
        last_b1 = b1;
    }
    return sink;
}
