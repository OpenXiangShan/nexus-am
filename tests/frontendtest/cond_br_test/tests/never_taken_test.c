#include <stdint.h>

#define LOOP_COUNT 1000

// Simple never-taken branch pattern
__attribute__((noinline))
int32_t never_taken(int32_t n) {
  int32_t sum = 0;
  for (int32_t i = 0; i < n; i++) {
    if (i < 0) { // Always false
      sum += i;
    } else {
      sum -= i;
    }

  }
  return sum;
}

int main() {
  never_taken(LOOP_COUNT);
  return 0;
} 