#include <klib.h>

static volatile int sink = 0;
static const int pattern[8] = {1,1,0,1,0,0,1,0};

int main(void) {
    int noise = 0;
    for (int i = 0; i < 5000; i++) {
        // 噪声分支，扰乱全局历史
        if ((i & 7) == 3) noise++;
        if ((i & 15) == 7) noise--;

        int bit = pattern[i & 7];
        if (bit) {
            sink += i;
        } else {
            sink -= i;
        }
    }
    return sink + noise;
}
