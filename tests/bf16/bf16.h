#ifndef BF16_H
#define BF16_H

#include <klib.h>
#include <math.h>
#include <stdint.h>

#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_RESET "\033[0m"

/*
** The value in the BASE field of mtvec
** must always be aligned on a 4-byte boundary
** aligned(4) means aligned on a 4-byte boundary
** not aligned on a 2^4 byte boundary
*/
extern __attribute__((aligned(4))) void __am_asm_trap(void);

#define INIT()                                                                 \
  do {                                                                         \
    asm volatile("lui a0,0x2\n"                                                \
                 "addiw a0,a0,512\n"                                           \
                 "csrs mstatus,a0\n"                                           \
                 "csrwi vcsr,0" ::);                                           \
  } while (0)

// Test data structure for BF16 conversion tests
typedef struct {
  float input;       // Input float value
  uint16_t expected; // Expected BF16 value
  const char *desc;  // Description of the test case
} test_case_t;

// Test data for BF16 conversion
extern test_case_t test_cases[];
extern int num_test_cases;

// Assembly implementations
typedef union {
  float f;
  uint32_t u;
} float_bits;

// Function prototypes
uint16_t float_to_bf16(float f);
float bf16_to_float(uint16_t bf);
void store_half(uint16_t *ptr, float val);
float load_half(const uint16_t *ptr);
uint32_t fmv_x_h(float half_in_fpr);
float fmv_h_x(uint32_t half_bits);

// Software implementations
uint16_t float_to_bf16_soft(float f);
float bf16_to_float_soft(uint16_t bf);
void store_half_soft(uint16_t *ptr, float val);
float load_half_soft(const uint16_t *ptr);
uint32_t fmv_x_h_soft(float half_in_fpr);
float fmv_h_x_soft(uint32_t half_bits);

/* Check if single-precision float is infinity */
int is_inf(float x);

/* Check if single-precision float is NaN */
int is_nan(float x);

/* Get sign bit of single-precision float (0 for positive, 1 for negative) */
int get_sign(float x);
float my_fabs(float x);

/* Compare two floats for equality (handle special values, consider tolerance)
 */
int float_equal(float a, float b, float abs_tol, float rel_tol);

/* ------------------------------------------------------------------------- */
/* bfloat16 utility functions */

/* Check if bf16 is infinity */
int bf16_is_inf(uint16_t bf);

/* Check if bf16 is NaN */
int bf16_is_nan(uint16_t bf);

/* Get sign bit of bf16 */
int bf16_get_sign(uint16_t bf);

/* ------------------------------------------------------------------------- */
/* Test functions */

// Test function prototypes
void test_fmv();
void test_store_load_half();
void test_float_to_bf16();
void test_vfncvtbf16();
void test_vfwcvtbf16();
void test_vfwmaccbf16();
void test_vfwmaccbf16_vf();

#endif