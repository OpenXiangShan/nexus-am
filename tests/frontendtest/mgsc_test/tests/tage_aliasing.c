#include <klib.h>

static volatile int sink;

// Force multiple branches to alias in TAGE index but differ in local behavior
// Use inline to keep branches close in PC space
static inline void __attribute__((always_inline)) branch_cluster(int loop, int offset) {
    // Branch 0: period 8, offset 0
    if (((loop + offset) & 7) < 3) sink += 1;

    // Branch 1: period 8, offset 2 (different phase)
    if (((loop + offset + 2) & 7) < 3) sink += 2;

    // Branch 2: period 8, offset 5 (different phase)
    if (((loop + offset + 5) & 7) < 3) sink += 3;

    // Branch 3: period 8, offset 7 (different phase)
    if (((loop + offset + 7) & 7) < 3) sink += 4;
}

int main(void) {
    // Run multiple times with different offsets to create aliasing
    for (int outer = 0; outer < 200; outer++) {
        for (int inner = 0; inner < 40; inner++) {
            int loop = outer * 40 + inner;
            // Use outer as offset to create different patterns
            branch_cluster(loop, outer & 7);
        }
    }
    return sink;
}
