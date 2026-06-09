#include "bf16.h"

// Assembly implementations
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

void set_rounding_mode(int rm) {
  asm volatile("csrw frm, %0" : : "r"(rm) : "memory");
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
  return float_to_bf16_soft_rm(f, 0);
}

uint16_t float_to_bf16_soft_rm(float f, int rm) {
  float_bits fb;
  fb.f = f;
  uint32_t abs = fb.u & 0x7FFFFFFF;
  uint32_t hi = fb.u >> 16;
  uint32_t lo = fb.u & 0xFFFF;
  uint32_t sign = fb.u >> 31;
  int increment = 0;

  if ((abs & 0x7F800000) == 0x7F800000) {
    uint16_t bf = (uint16_t)(fb.u >> 16);
    if (abs & 0x007FFFFF) {
      bf |= 0x0040; // Preserve NaN while quieting signaling NaNs.
    }
    return bf;
  }

  switch (rm) {
  case 0: // RNE: round to nearest, ties to even
    increment = (lo > 0x8000) || ((lo == 0x8000) && (hi & 1));
    break;
  case 1: // RTZ: round toward zero
    increment = 0;
    break;
  case 2: // RDN: round down toward -inf
    increment = sign && (lo != 0);
    break;
  case 3: // RUP: round up toward +inf
    increment = !sign && (lo != 0);
    break;
  case 4: // RMM: round to nearest, ties to max magnitude
    increment = lo >= 0x8000;
    break;
  default:
    increment = (lo > 0x8000) || ((lo == 0x8000) && (hi & 1));
    break;
  }

  return (uint16_t)(hi + increment);
}

float bf16_to_float_soft(uint16_t bf) {
  float_bits fb;
  fb.u = (uint32_t)bf << 16;
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

/* Check if single-precision float is infinity */
int is_inf(float x) {
  float_bits fb = {.f = x};
  uint32_t exp = (fb.u >> 23) & 0xFF;
  uint32_t mant = fb.u & 0x7FFFFF;
  return (exp == 0xFF) && (mant == 0);
}

/* Check if single-precision float is NaN */
int is_nan(float x) {
  float_bits fb = {.f = x};
  uint32_t exp = (fb.u >> 23) & 0xFF;
  uint32_t mant = fb.u & 0x7FFFFF;
  return (exp == 0xFF) && (mant != 0);
}

/* Get sign bit of single-precision float (0 for positive, 1 for negative) */
int get_sign(float x) {
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
int float_equal(float a, float b, float abs_tol, float rel_tol) {
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
int bf16_is_inf(uint16_t bf) {
  uint16_t exp = (bf >> 7) & 0xFF;
  uint16_t mant = bf & 0x7F;
  return (exp == 0xFF) && (mant == 0);
}

/* Check if bf16 is NaN */
int bf16_is_nan(uint16_t bf) {
  uint16_t exp = (bf >> 7) & 0xFF;
  uint16_t mant = bf & 0x7F;
  return (exp == 0xFF) && (mant != 0);
}

/* Get sign bit of bf16 */
int bf16_get_sign(uint16_t bf) { return (bf >> 15) & 0x1; }

int bf16_matches_float_conversion(float input, uint16_t actual,
                                  uint16_t expected) {
  if (is_nan(input)) {
    return bf16_is_nan(actual);
  }
  if (is_inf(input)) {
    return bf16_is_inf(actual) && (bf16_get_sign(actual) == get_sign(input));
  }
  return actual == expected;
}

int float_matches_bf16_conversion(uint16_t input, float actual) {
  if (bf16_is_nan(input)) {
    return is_nan(actual);
  }

  float_bits actual_bits;
  actual_bits.f = actual;
  return actual_bits.u == ((uint32_t)input << 16);
}
