#include <klib.h>

static volatile int sink = 0;

int main(void) {
    // Multiple nested loops with different iteration-dependent patterns
    for (int outer = 0; outer < 200; outer++) {
        // Inner loop 1: iteration count matters
        for (int j = 0; j < 16; j++) {
            // Branch depends on iteration AND outer loop phase
            // TAGE might learn iteration pattern, but SC refines with outer context
            if ((j == 7) || (j == 13 && (outer & 3) == 2)) {
                sink ^= outer;
            } else {
                sink += j;
            }
        }

        // Inner loop 2: different iteration pattern
        for (int k = 0; k < 12; k++) {
            // Multiple iteration-dependent branches
            if (k < 4) sink++;
            if (k >= 8) sink--;

            // This branch needs both iteration count AND global context
            if ((k & 3) == (outer & 3)) {
                sink += 2;
            }
        }

        // Add noise to global history
        if ((outer & 7) == 5) sink++;
    }
    return sink;
}
