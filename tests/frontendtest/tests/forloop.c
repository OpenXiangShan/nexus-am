#include <stdio.h>

void three_level_loop_test(int outer_iterations, int middle_iterations, int inner_iterations) {
    volatile int result = 0;

    for (int i = 0; i < outer_iterations; i++) {
        for (int j = 0; j < middle_iterations; j++) {
            for (int k = 0; k < inner_iterations; k++) {
                // Perform some operation to test the loop
                result += (i + j + k);
            }
        }
    }

    // printf("Result: %d\n", result);
}

int main() {
    // Run the three-level loop test with specified iterations
    for (int i = 0; i < 10; i++) {
        three_level_loop_test(10, 10, 10);
    }
    return 0;
}
