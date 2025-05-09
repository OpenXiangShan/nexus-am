#include <klib.h>

// Array of function pointers for indirect branches
typedef void (*func_ptr)(int*);

// Four different target functions with different behaviors
void __attribute__((noinline)) target1(int* counter) {
    (*counter) += 1;
}

void __attribute__((noinline)) target2(int* counter) {
    (*counter) += 2;
}

void __attribute__((noinline)) target3(int* counter) {
    (*counter) += 3;
}

void __attribute__((noinline)) target4(int* counter) {
    (*counter) += 4;
}

// Test for indirect branch target prediction using function pointers
void __attribute__((noinline)) indirect_branch_test(int iterations) {
    int result = 0;
    
    // Initialize array of function pointers
    func_ptr targets[4];
    targets[0] = target1;
    targets[1] = target2;
    targets[2] = target3;
    targets[3] = target4;
    
    // Create a pattern of indirect branches
    for (int i = 0; i < iterations; i++) {
        // Choose target based on a pattern
        int target_idx = i % 4;
        
        // Call the function through pointer (indirect branch)
        func_ptr target_func = targets[target_idx];
        target_func(&result);
    }
}

int main() {
    // Run enough iterations to train the branch predictor
    indirect_branch_test(1000);
    return 0;
} 