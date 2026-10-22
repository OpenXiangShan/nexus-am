#include <klib.h>
static volatile int sink;

int main(void) {
    // Reduce period to be within TAGE range but still challenging
    const int period = 47;   // Prime number, harder to learn

    for (int i = 0; i < 6000; i++) {
        // Add multiple branches with related but different periods
        // This creates interference that SC can help resolve

        // Branch 1: period 47
        if ((i % period) < 12) {
            sink += 1;
        } else {
            sink -= 1;
        }

        // Branch 2: period 43 (different prime)
        if ((i % 43) < 10) {
            sink += 2;
        }

        // Branch 3: depends on interaction of above periods
        // TAGE sees mixed global history, SC can use multiple tables
        int phase = (i / period) & 3;
        if (((i % period) < 12) ^ (phase & 1)) {
            sink += 3;
        } else {
            sink -= 2;
        }

        // Noise branches
        if ((i & 7) == 3) sink--;
        if ((i & 15) == 7) sink++;
    }
    return sink;
}
