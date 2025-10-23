#include <stdint.h>

#define LOOP_COUNT 1000

// 3-bit pattern (TTTNNNTTTNNNT...)
__attribute__((noinline))
int32_t three_bit_pattern(int32_t n) {
  int32_t sum = 0;
  for (int32_t i = 0; i < n; i++) {
    if ((i / 3) % 2 == 0) { // TTT NNN TTT pattern
      sum += i;
    } else {
      sum -= i;
    }

  }
  return sum;
}

int main() {
  three_bit_pattern(LOOP_COUNT);
  return 0;
} 