#include <klib.h>

static volatile int sink;

// Pattern that requires combining multiple history dimensions
// Global + Local + Path all contribute
int main(void) {
    int local_state = 0;

    for (int i = 0; i < 5000; i++) {
        // Create global history pattern
        int g_bit = ((i >> 2) & 1);  // slow changing
        if (g_bit) sink++;

        // Create local state that evolves
        local_state = (local_state + 1) & 15;

        // Branch 1: depends on global AND local
        // TAGE might capture global, but SC needs to refine with local
        if ((g_bit && (local_state < 8)) || (!g_bit && (local_state >= 8))) {
            sink += 2;
        } else {
            sink -= 1;
        }

        // Noise branches to pollute global history
        if ((i & 7) == 5) sink++;
        if ((i & 15) == 11) sink--;

        // Branch 2: XOR of multiple factors
        int b2 = ((i & 3) == 0) ^ ((local_state & 3) == 2);
        if (b2) {
            sink += 3;
        } else {
            sink -= 2;
        }

        // Branch 3: depends on recent branch outcomes
        int recent = (g_bit << 1) | b2;
        if (recent == 2 || recent == 1) {  // non-linear combination
            sink += 4;
        }
    }
    return sink;
}
