#include <stdint.h>

#define LOOP_COUNT 1000

// Nested branch patterns that might confuse branch predictors
__attribute__((noinline))
int32_t nested_branches(int32_t n) {
  int32_t sum = 0;
  for (int32_t i = 0; i < n; i++) {
    if (i % 3 == 0) {
      if (i % 5 == 0) {
        sum += i * 2;
      } else {
        sum += i;
      }
    } else {
      if (i % 7 == 0) {
        sum -= i * 2;
      } else {
        sum -= i;
      }

    }

  }
  return sum;
}

int main() {
  nested_branches(LOOP_COUNT);
  return 0;
} 