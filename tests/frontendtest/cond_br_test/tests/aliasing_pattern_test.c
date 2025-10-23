#include <stdint.h>

#define LOOP_COUNT 1000

// Pattern designed to exploit aliasing in branch predictor tables
__attribute__((noinline))
int32_t aliasing_pattern(int32_t n) {
  int32_t sum = 0;
  
  // First pass - train the predictor with one pattern
  for (int32_t i = 0; i < n; i++) {
    // Use addresses that might alias in the branch predictor
    if (i % 16 < 8) {
      sum += i;
    } else {
      sum -= i;
    }
  }
  
  // Second pass - use a different pattern at similar addresses
  for (int32_t i = 0; i < n; i += 16) {
    // This creates potential aliasing with the first pattern
    if ((i + 64) % 16 >= 8) { // Inverted pattern from first loop
      sum += i;
    } else {
      sum -= i;
    }
  }
  
  return sum;
}

int main() {
  aliasing_pattern(LOOP_COUNT);
  return 0;
} 