#include <klib.h>

static volatile int sink;

// Create patterns where individual TAGE tables have weak but correct signals
// SC weight scaling should amplify the right signals
int main(void) {
    int state_a = 0, state_b = 0, state_c = 0;

    for (int i = 0; i < 6000; i++) {
        // Update multiple state machines with different periods
        state_a = (state_a + 1) % 11;  // period 11
        state_b = (state_b + 1) % 13;  // period 13
        state_c = (state_c + 1) % 17;  // period 17

        // Branch 1: weak correlation with state_a
        // Correct 60% of time based on state_a
        if ((state_a < 6) ^ ((i >> 3) & 1)) {
            sink += 1;
        }

        // Branch 2: weak correlation with state_b
        if ((state_b < 7) ^ ((i >> 4) & 1)) {
            sink += 2;
        }

        // Target branch: needs to combine weak signals from multiple sources
        // TAGE tables will have weak correlations
        // SC should weight them properly to get correct prediction
        int signal_a = (state_a < 6) ? 1 : 0;
        int signal_b = (state_b < 7) ? 1 : 0;
        int signal_c = (state_c < 9) ? 1 : 0;

        // Majority vote of weak signals
        int vote = signal_a + signal_b + signal_c;
        if (vote >= 2) {
            sink += 5;
        } else {
            sink -= 3;
        }

        // Add some noise
        if ((i & 15) == 9) sink++;
        if ((i & 31) == 21) sink--;
    }
    return sink;
}
