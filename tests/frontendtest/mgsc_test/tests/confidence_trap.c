#include <klib.h>

static volatile int sink;

// Create a pattern where TAGE builds high confidence but is occasionally wrong
// SC should catch these high-confidence mistakes
int main(void) {
    for (int phase = 0; phase < 8; phase++) {
        // Each phase: 99 times same direction, then 1 flip
        for (int i = 0; i < 100; i++) {
            int loop = phase * 100 + i;

            // Main branch: mostly predictable, rare flips
            // TAGE will be confident but occasionally wrong
            int flip = (i == 99);  // flip at end of each phase

            // Add some global history noise
            if ((loop & 3) == 0) sink++;
            if ((loop & 7) == 3) sink--;

            // Target branch: needs SC to catch the rare flips
            if (flip ^ (phase & 1)) {
                sink += 5;
            } else {
                sink += 1;
            }

            // Another branch with different flip pattern
            if ((i % 33) == 0) {  // period 33
                sink ^= loop;
            } else {
                sink += 2;
            }
        }
    }
    return sink;
}
