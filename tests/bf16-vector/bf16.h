#ifndef BF16_H
#define BF16_H

#include <stdint.h>

#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_RESET "\033[0m"

// Assembly implementations
typedef union {
  float f;
  uint32_t u;
} float_bits;


float bf16_to_float(uint16_t bf) {
  float f;
  asm volatile("fmv.h.x ft0, %1\n"
               "fcvt.s.bf16 %0, ft0"
               // "fmv.s %0, ft0"
               : "=f"(f)
               : "r"(bf)
               : "ft0");
  return f;
}

/* Check if single-precision float is infinity */
static int is_inf(float x) {
  float_bits fb = {.f = x};
  uint32_t exp = (fb.u >> 23) & 0xFF;
  uint32_t mant = fb.u & 0x7FFFFF;
  return (exp == 0xFF) && (mant == 0);
}

/* Check if single-precision float is NaN */
static int is_nan(float x) {
  float_bits fb = {.f = x};
  uint32_t exp = (fb.u >> 23) & 0xFF;
  uint32_t mant = fb.u & 0x7FFFFF;
  return (exp == 0xFF) && (mant != 0);
}

/* Get sign bit of single-precision float (0 for positive, 1 for negative) */
static int get_sign(float x) {
  float_bits fb = {.f = x};
  return (fb.u >> 31) & 0x1;
}
float my_fabs(float x) {
  union {
    float f;
    uint32_t u;
  } u = {.f = x};
  u.u &= 0x7fffffff; // Clear sign bit (highest bit)
  return u.f;
}
/* Compare two floats for equality (handle special values, consider tolerance)
 */
static int float_equal(float a, float b, float abs_tol, float rel_tol) {
  /* Handle NaN */
  if (is_nan(a) || is_nan(b)) {
    return is_nan(a) && is_nan(b);
  }
  /* Handle infinity */
  if (is_inf(a) || is_inf(b)) {
    return is_inf(a) && is_inf(b) && (get_sign(a) == get_sign(b));
  }
  /* Normal numbers: use absolute and relative error */
  float diff = a - b;
  if (diff < 0)
    diff = -diff;
  if (diff <= abs_tol)
    return 1;
  float max_abs = a;
  if (b > a)
    max_abs = b;
  if (max_abs < 0)
    max_abs = -max_abs;
  if (max_abs < 1e-10f)
    max_abs = 1.0f; /* Avoid division by zero */
  if (diff / max_abs <= rel_tol)
    return 1;
  return 0;
}

#endif