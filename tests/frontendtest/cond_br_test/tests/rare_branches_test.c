#include <stdint.h>

#define LOOP_COUNT 1000

// Rare branch pattern (branch taken very infrequently)
__attribute__((noinline))
int32_t rare_branches(int32_t n) {
  int32_t sum = 0;
  for (int32_t i = 0; i < n; i++) {
    // Branch only taken about 1% of the time
    if (i % 100 == 97) {
      sum += i * 10;  // Significant impact when taken
    } else {
      sum += i;
    }

  }
  return sum;
}

int main() {
  rare_branches(LOOP_COUNT);
  return 0;
} 