#include <klib.h>

// Test for unconditional jump target prediction using goto statements
void __attribute__((noinline)) jump_branch_test(int iterations) {
    volatile int result = 0;
    
    // Create a pattern of jumps to different targets
    for (int i = 0; i < iterations; i++) {
        // Choose jump target based on a pattern
        int jump_target = i % 4;
        
        // Use goto (unconditional jump) to different targets
        switch (jump_target) {
            case 0:
                goto target1;
            case 1:
                goto target2;
            case 2:
                goto target3;
            case 3:
                goto target4;
        }
        
        // Jump targets with different operations
    target1:
        result += 1;
        goto continue_loop;
        
    target2:
        result += 2;
        goto continue_loop;
        
    target3:
        result += 3;
        goto continue_loop;
        
    target4:
        result += 4;
        
    continue_loop:
        // Loop continues here
        continue;
    }
}

int main() {
    // Run enough iterations to train the branch predictor
    jump_branch_test(1000);
    return 0;
} 