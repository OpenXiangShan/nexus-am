#include <klib.h>

// A simple function that will be called from different sites
int __attribute__((noinline)) func_for_ret_test(int value) {
    // The return instruction is what we're testing
    return value;
}

// Test for return branch target prediction
void __attribute__((noinline)) return_branch_test(int iterations) {
    volatile int result = 0;
    
    // Create a pattern of calls to the same function from different sites
    for (int i = 0; i < iterations; i++) {
        // Decide which call site to use based on iteration
        int call_site = i % 4;
        
        switch (call_site) {
            case 0:
                // Call site 1
                result += func_for_ret_test(1);
                break;
            case 1:
                // Call site 2
                result += func_for_ret_test(2);
                break;
            case 2:
                // Call site 3
                result += func_for_ret_test(3);
                break;
            case 3:
                // Call site 4
                result += func_for_ret_test(4);
                break;
        }
    }
}

int main() {
    // Run enough iterations to train the return address predictor
    return_branch_test(1000);
    return 0;
} 