#include <stdint.h>

#define LOOP_COUNT 1000

// Random-looking but deterministic pattern using prime numbers
__attribute__((noinline))
int32_t prime_based_pattern(int32_t n) {
  int32_t sum = 0;
  for (int32_t i = 0; i < n; i++) {
    // Creates a less predictable pattern but still deterministic
    if ((i * 7 + 13) % 19 < 10) { 
      sum += i;
    } else {
      sum -= i;
    }

  }
  return sum;
}

int main() {
  prime_based_pattern(LOOP_COUNT);
  return 0;
} 