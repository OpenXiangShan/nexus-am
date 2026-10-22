#include <klib.h>

static volatile int sink;

#define HIST_LEN 12
static int hist[HIST_LEN];

static inline unsigned lfsr_next(unsigned s) {
    // 16bit LFSR，保持确定性但看起来随机
    return (s >> 1) ^ (-(s & 1u) & 0xB400u);
}

int main(void) {
    unsigned lfsr = 1;
    for (int i = 0; i < 7000; i++) {
        lfsr = lfsr_next(lfsr);
        int bit = (lfsr & 1u) ? 1 : -1;   // 伪随机符号

        hist[i % HIST_LEN] = bit;

        int sum = 0;
        for (int j = 0; j < HIST_LEN; j++) {
            sum += hist[j];
        }

        // 目标分支：做“多数投票”，线性可分，利于感知机类修正器
        if (sum >= 0) {
            sink += 3;
        } else {
            sink -= 2;
        }
    }
    return sink;
}
