#include <klib.h>

static volatile int sink;

// Test where bias table is critical for SC decision
// Create branches with strong bias but occasional exceptions
int main(void) {
    for (int epoch = 0; epoch < 50; epoch++) {
        // Each epoch has a dominant direction with rare exceptions
        int bias_dir = epoch & 1;  // alternating bias per epoch

        for (int i = 0; i < 120; i++) {
            int idx = epoch * 120 + i;

            // Create complex global history
            if ((idx & 3) == 0) sink++;
            if ((idx & 7) == 5) sink--;
            if ((idx & 15) == 11) sink++;

            // Branch 1: strong bias (95% one direction) but flips between epochs
            // TAGE might be confused by global history
            // SC bias table should capture the per-epoch bias
            int exception = (i == 5) || (i == 37) || (i == 89) || (i == 103);
            if (bias_dir ^ exception) {
                sink += 3;
            } else {
                sink -= 2;
            }

            // Branch 2: another biased branch with different exception pattern
            int exception2 = (i % 29) == 0;  // every 29 iterations
            if (bias_dir ^ exception2) {
                sink += 4;
            } else {
                sink -= 1;
            }

            // Branch 3: depends on both above branches' bias
            // Needs SC to combine bias information
            if ((bias_dir && !exception) || (!bias_dir && exception2)) {
                sink += 5;
            }
        }
    }
    return sink;
}
