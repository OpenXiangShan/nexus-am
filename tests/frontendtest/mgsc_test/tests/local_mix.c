#include <klib.h>

static volatile int sink;

// 多个静态分支，各自有短周期本地模式，交错执行以干扰全局历史
static const int pat0[] = {1, 1, 0};                   // len 3
static const int pat1[] = {1, 0, 0, 1};                // len 4
static const int pat2[] = {0, 1, 0, 1, 1};             // len 5
static const int pat3[] = {1, 1, 1, 0, 0, 0};          // len 6
static const int pat4[] = {1, 0, 1, 0, 1, 0, 0};       // len 7
static const int pat5[] = {0, 0, 1, 1, 0, 1, 1, 0};    // len 8

static const int *patterns[] = {pat0, pat1, pat2, pat3, pat4, pat5};
static const int lens[] = {3, 4, 5, 6, 7, 8};
static int idx[6];

int main(void) {
    for (int loop = 0; loop < 4000; loop++) {
        for (int s = 0; s < 6; s++) {
            int bit = patterns[s][idx[s]];
            idx[s] = (idx[s] + 1) % lens[s];

            if (bit) {
                sink += (s + 1);
            } else {
                sink -= (s + 1);
            }
        }
    }
    return sink;
}
