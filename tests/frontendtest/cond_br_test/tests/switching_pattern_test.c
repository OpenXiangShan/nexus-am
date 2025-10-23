#include <stdint.h>

#define LOOP_COUNT 1000
#define PATTERN_SWITCH 500

// Pattern that switches from mostly taken to mostly not taken
__attribute__((noinline))
int32_t switching_pattern(int32_t n, int32_t switch_point) {
  int32_t sum = 0;
  for (int32_t i = 0; i < n; i++) {
    if (i < switch_point) {
      // First half: 80% taken
      if (i % 10 < 8) {
        sum += i;
      } else {
        sum -= i;
      }
    } else {
      // Second half: 20% taken
      if (i % 10 < 2) {
        sum += i;
      } else {
        sum -= i;
      }
    }
  }
  return sum;
}

int main() {
  switching_pattern(LOOP_COUNT, PATTERN_SWITCH);
  return 0;
} 