#include <stdint.h>

#define LOOP_COUNT 1000

// Gradually changing pattern from always taken to never taken
__attribute__((noinline))
int32_t gradual_transition(int32_t n) {
  int32_t sum = 0;
  for (int32_t i = 0; i < n; i++) {
    // Gradually changes from 100% taken to 0% taken
    if (i * 100 / n < (100 - i * 100 / n)) {
      sum += i;
    } else {
      sum -= i;
    }

  }
  return sum;
}

int main() {
  gradual_transition(LOOP_COUNT);
  return 0;
} 