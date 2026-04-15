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

uint16_t float_to_bf16(float f) {
  uint16_t bf;
  // uint32_t bf;
  asm volatile("fcvt.bf16.s ft0, %1\n"
               "fmv.x.h %0, ft0"
               : "=r"(bf)
               : "f"(f)
               : "ft0");
  return (uint16_t)bf;
}
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
void store_half(uint16_t *ptr, float val) {
  asm volatile("fsh %1, 0(%0)"
               :
               : "r"(ptr),  // Input: pointer register
                 "f"(val)); // Input: floating-point value);
}

float load_half(const uint16_t *ptr) {
  float val;
  asm volatile("flh %0, 0(%1)\n"
               : "=f"(val)  // Output: floating-point register
               : "r"(ptr)); // Input: pointer register);
  return val;
}

uint32_t fmv_x_h(float half_in_fpr) {
  uint32_t result;
  asm volatile("fmv.x.h %0, %1"
               : "=r"(result)     // Output: integer register
               : "f"(half_in_fpr) // Input: floating-point register
  );
  return result;
}

float fmv_h_x(uint32_t half_bits) {
  float result;
  asm volatile("fmv.h.x %0, %1"
               : "=f"(result)   // Output: floating-point register
               : "r"(half_bits) // Input: integer register (lower 16 bits valid)
  );
  return result;
}

// Software implementations
uint16_t float_to_bf16_soft(float f) {
  float_bits fb;
  fb.f = f;
  uint32_t sign = (fb.u >> 31) & 0x1;
  uint32_t exponent = (fb.u >> 23) & 0xFF;
  uint32_t mantissa = fb.u & 0x7FFFFF;

  if (exponent == 0xFF) {
    // NaN or infinity
    return (sign << 15) | 0x7F80 | (mantissa >> 16);
  } else if (exponent == 0) {
    // Zero or denormal
    return (sign << 15);
  }

  // Normal number: round to nearest even
  uint32_t rounding = (mantissa >> 15) & 1; // Highest bit of discarded part
  uint32_t sticky =
      (mantissa & 0x7FFF) != 0; // Whether discarded part has non-zero bits
  mantissa = (mantissa >> 16) + (rounding & sticky); // Carry
  // Handle exponent overflow due to carry (very rare case)
  if (mantissa == 0x80) {
    mantissa = 0;
    exponent++;
  }
  return (sign << 15) | (exponent << 7) | mantissa;
}

float bf16_to_float_soft(uint16_t bf) {
  float_bits fb;
  uint32_t sign = (bf >> 15) & 0x1;
  uint32_t exponent = (bf >> 7) & 0xFF;
  uint32_t mantissa = bf & 0x7F;

  if (exponent == 0xFF) {
    // NaN or infinity
    fb.u = (sign << 31) | 0x7F800000 | (mantissa << 16);
  } else if (exponent == 0) {
    // Denormal or zero: preserve mantissa
    fb.u = (sign << 31) | (mantissa << 16);
  } else {
    // Normal case
    fb.u = (sign << 31) | (exponent << 23) | (mantissa << 16);
  }
  return fb.f;
}

void store_half_soft(uint16_t *ptr, float val) {
  *ptr = float_to_bf16_soft(val);
}

float load_half_soft(const uint16_t *ptr) { return bf16_to_float_soft(*ptr); }

uint32_t fmv_x_h_soft(float half_in_fpr) {
  return (uint32_t)float_to_bf16_soft(half_in_fpr);
}

float fmv_h_x_soft(uint32_t half_bits) {
  return bf16_to_float_soft((uint16_t)half_bits);
}

static float make_inf(int sign) {
  float_bits fb;
  fb.u = (sign ? 0xFF800000 : 0x7F800000);
  return fb.f;
}

static float make_nan(void) {
  float_bits fb;
  fb.u = 0x7FC00000; /* quiet NaN, mantissa highest bit is 1 */
  return fb.f;
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

/* ------------------------------------------------------------------------- */
/* bfloat16 utility functions */

/* Check if bf16 is infinity */
static int bf16_is_inf(uint16_t bf) {
  uint16_t exp = (bf >> 7) & 0xFF;
  uint16_t mant = bf & 0x7F;
  return (exp == 0xFF) && (mant == 0);
}

/* Check if bf16 is NaN */
static int bf16_is_nan(uint16_t bf) {
  uint16_t exp = (bf >> 7) & 0xFF;
  uint16_t mant = bf & 0x7F;
  return (exp == 0xFF) && (mant != 0);
}

/* Get sign bit of bf16 */
static int bf16_get_sign(uint16_t bf) { return (bf >> 15) & 0x1; }

/* ------------------------------------------------------------------------- */
/* Test functions */

#endif