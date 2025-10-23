#include <stdint.h>

#define LOOP_COUNT 1000

// 2-bit pattern (TTNNTTNN...)
__attribute__((noinline))
int32_t two_bit_pattern(int32_t n) {
  int32_t sum = 0;
  for (int32_t i = 0; i < n; i++) {
    if ((i / 2) % 2 == 0) { // TT NN TT NN pattern
      sum += i;
    } else {
      sum -= i;
    }
  }
  return sum;
}

int main() {
  two_bit_pattern(LOOP_COUNT);
  return 0;
} 