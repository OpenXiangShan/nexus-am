#include <stdint.h>

#define LOOP_COUNT 1000

// Loop with early exits pattern (branch predictor needs to predict the exit condition)
__attribute__((noinline))
int32_t early_exits(int32_t n) {
  int32_t sum = 0;
  int32_t limit = n * 2;
  
  for (int32_t i = 0; i < limit; i++) {
    sum += i;
    // Exit early with a pattern
    if ((i > n/2) && (i % 17 == 0)) {
      break;
    }
  }
  return sum;
}

int main() {
  early_exits(LOOP_COUNT);
  return 0;
} 