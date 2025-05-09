#include <klib.h>

// Four different functions to call for testing call branch prediction
int __attribute__((noinline)) func1(int x) {
    return x + 1;
}

int __attribute__((noinline)) func2(int x) {
    return x + 2;
}

int __attribute__((noinline)) func3(int x) {
    return x + 3;
}

int __attribute__((noinline)) func4(int x) {
    return x + 4;
}

// Test for call branch target prediction
void __attribute__((noinline)) call_branch_test(int iterations) {
    volatile int result = 0;
    
    // Create a pattern of calls to different functions
    for (int i = 0; i < iterations; i++) {
        // Use a 16-iteration pattern
        int pattern_pos = i % 16;
        
        if (pattern_pos < 4) {
            // First pattern: call func1 for iterations 0-3
            result += func1(i);
        } else if (pattern_pos < 8) {
            // Second pattern: call func2 for iterations 4-7
            result += func2(i);
        } else if (pattern_pos < 12) {
            // Third pattern: call func3 for iterations 8-11
            result += func3(i);
        } else {
            // Fourth pattern: call func4 for iterations 12-15
            result += func4(i);
        }
    }
}

int main() {
    // Run enough iterations to train the branch predictor
    call_branch_test(1000);
    return 0;
} 