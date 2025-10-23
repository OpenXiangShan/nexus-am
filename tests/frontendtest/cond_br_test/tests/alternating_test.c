#include <stdint.h>

#define LOOP_COUNT 1000

// Regular alternating pattern (TNTNTNT...)
__attribute__((noinline))
int32_t alternating(int32_t n) {
  int32_t sum = 0;
  for (int32_t i = 0; i < n; i++) {
    if (i % 2 == 0) { // Alternates true/false
      sum += i;
    } else {
      sum -= i;
    }

  }
  return sum;
}

int main() {
  alternating(LOOP_COUNT);
  return 0;
} 