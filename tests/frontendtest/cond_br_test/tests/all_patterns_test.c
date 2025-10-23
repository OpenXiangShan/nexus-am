#ifndef BRANCH_PATTERN_COMMON_H
#define BRANCH_PATTERN_COMMON_H

#include <stdint.h>

#define LOOP_COUNT 1000
#define PATTERN_SWITCH 500

// Simple always-taken branch pattern
__attribute__((noinline))
int32_t always_taken(int32_t n) {
  int32_t sum = 0;
  for (int32_t i = 0; i < n; i++) {
    if (i >= 0) { // Always true
      sum += i;
    } else {
      sum -= i;
    }
  }
  return sum;
}

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

// Pattern designed to exploit aliasing in branch predictor tables
__attribute__((noinline))
int32_t aliasing_pattern(int32_t n) {
  int32_t sum = 0;
  
  // First pass - train the predictor with one pattern
  for (int32_t i = 0; i < n; i++) {
    // Use addresses that might alias in the branch predictor
    if (i % 16 < 8) {
      sum += i;
    } else {
      sum -= i;
    }
  }
  
  // Second pass - use a different pattern at similar addresses
  for (int32_t i = 0; i < n; i += 16) {
    // This creates potential aliasing with the first pattern
    if ((i + 64) % 16 >= 8) { // Inverted pattern from first loop
      sum += i;
    } else {
      sum -= i;
    }
  }
  
  return sum;
}
int main() {
  int32_t result = 0;
  
  // Run each test pattern
  result += always_taken(LOOP_COUNT);
  result += never_taken(LOOP_COUNT);
  result += alternating(LOOP_COUNT);
  result += two_bit_pattern(LOOP_COUNT);
  result += three_bit_pattern(LOOP_COUNT);
  result += prime_based_pattern(LOOP_COUNT);
  result += switching_pattern(LOOP_COUNT, PATTERN_SWITCH);
  result += nested_branches(LOOP_COUNT);
  result += early_exits(LOOP_COUNT);
  result += rare_branches(LOOP_COUNT);
  result += gradual_transition(LOOP_COUNT);
  result += aliasing_pattern(LOOP_COUNT);
  
  return 0;
}

#endif // BRANCH_PATTERN_COMMON_H
