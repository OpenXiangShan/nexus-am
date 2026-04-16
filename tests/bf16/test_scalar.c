#include "bf16.h"

// Test data for BF16 conversion
test_case_t test_cases[] = {
    // Normal numbers
    {1.0f, 0x3F80, "1.0"},
    {0.0f, 0x0000, "0.0"},
    {-1.0f, 0xBF80, "-1.0"},
    {0.5f, 0x3F00, "0.5"},
    {-0.5f, 0xBF00, "-0.5"},
    {2.0f, 0x4000, "2.0"},
    {-2.0f, 0xC000, "-2.0"},
    {100.0f, 0x42C8, "100.0"},
    {-100.0f, 0xC2C8, "-100.0"},
    {0.0001f, 0x38D2, "0.0001"},
    {-0.0001f, 0xB8D2, "-0.0001"},
    {100000.0f, 0x47C3, "100000.0"},
    {-100000.0f, 0xC7C3, "-100000.0"},

    // Special values
    {INFINITY, 0x7F80, "+inf"},
    {-INFINITY, 0xFF80, "-inf"},
    {NAN, 0x7FC0, "NaN"},

    // Denormal numbers
    {1.40129846e-45f, 0x0000, "Smallest positive denormal"},
    {1.0e-45f, 0x0000, "Rounds to 0"},
    {-1.40129846e-45f, 0x8000, "Smallest negative denormal"},

    // Edge cases
    {0.0078125f, 0x3C00, "2^-7"},
    {65504.0f, 0x4780, "Largest normal number"},
};

int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

void test_store_load_half() {
  printf("Testing store_half and load_half:\n");
  int cnt_fail_store = 0;
  int cnt_fail_load = 0;

  for (uint16_t i = 0; i < 0xFFFF; i++) {
    float_bits orig;
    orig.u = (uint32_t)i;
    float f = orig.f;

    uint16_t buffer_asm;
    store_half(&buffer_asm, f);

    float_bits f_loaded_asm;
    f_loaded_asm.f = load_half(&buffer_asm);

    if (buffer_asm != orig.u) {
      printf("store_half failed: 0x%04x != 0x%04x\n", buffer_asm, orig.u);
      cnt_fail_store++;
    }

    if ((uint16_t)f_loaded_asm.u != orig.u) {
      printf("load_half failed: 0x%04x != 0x%04x\n", (uint16_t)f_loaded_asm.u,
             orig.u);
      cnt_fail_load++;
    }
  }

  if (cnt_fail_store == 0) {
    printf("%sPASS%s: store_half passed all tests.\n", COLOR_GREEN,
           COLOR_RESET);
  } else {
    printf("%sFAIL%s: store_half failed %d/%d times.\n", COLOR_RED, COLOR_RESET,
           cnt_fail_store, 0xFFFF);
  }

  if (cnt_fail_load == 0) {
    printf("%sPASS%s: load_half passed all tests.\n\n", COLOR_GREEN,
           COLOR_RESET);
  } else {
    printf("%sFAIL%s: load_half failed %d/%d times.\n\n", COLOR_RED,
           COLOR_RESET, cnt_fail_load, 0xFFFF);
  }
}

void test_float_to_bf16(void) {
  printf("Testing float_to_bf16:\n");

  int pass_count = 0;

  for (int i = 0; i < num_test_cases; i++) {
    float f = test_cases[i].input;
    uint16_t expected = test_cases[i].expected;
    const char *desc = test_cases[i].desc;
    // uint16_t bf = float_to_bf16_soft(f);
    uint16_t bf = float_to_bf16(f);

    int ok = 0;

    /* Special handling for NaN: As long as output is NaN, it's correct
     * (mantissa doesn't need to match exactly) */
    if (is_nan(f)) {
      ok = bf16_is_nan(bf);
    } else if (is_inf(f)) {
      /* Infinity: Requires all 1s in exponent, 0s in mantissa, and matching
       * sign */
      ok = bf16_is_inf(bf) && (bf16_get_sign(bf) == get_sign(f));
    } else {
      /* Normal numbers: Direct bit pattern comparison */
      ok = (bf == expected);
    }

    if (ok) {
      printf("%sPASS%s: %s (%g) -> 0x%04x (expected 0x%04x)\n", COLOR_GREEN,
             COLOR_RESET, desc, f, bf, expected);
      pass_count++;
    } else {
      printf("%sFAIL%s: %s (%g) -> 0x%04x, expected 0x%04x\n", COLOR_RED,
             COLOR_RESET, desc, f, bf, expected);
    }
  }

  printf("Passed %d/%d tests.\n\n", pass_count, num_test_cases);

  /* --------------------------------------------------------------------- */
  /* Reverse conversion test (float -> bf16 -> float) */
  printf("Round-trip test (float -> bf16 -> float):\n");

  float roundtrip_vals[] = {
      1.0f, 0.0f, -0.0f, 0.5f, 0.0001f, 65504.0f, 1.40129846e-45f,
      // INFINITY, -INFINITY, NAN
  };
  int num_rt = sizeof(roundtrip_vals) / sizeof(float);
  int rt_pass = 0;

  for (int i = 0; i < num_rt; i++) {
    float orig = roundtrip_vals[i];
    uint16_t bf = float_to_bf16(orig);
    float back = bf16_to_float(bf);
    int ok = 0;

    if (is_nan(orig)) {
      ok = is_nan(back);
    } else if (is_inf(orig)) {
      ok = is_inf(back) && (get_sign(orig) == get_sign(back));
    } else {
      /* Normal numbers: Allow small absolute or relative errors */

      float ulp = my_fabs(orig) * (1.0f / 64.0f); // BF16 的步长 ≈ |x| / 2^7
      ok = (my_fabs(orig - back) <= 2.0f * ulp) ||
           float_equal(orig, back, 1e-6f, 1e-5f);
      ;
    }

    if (ok) {
      printf("%sPASS%s: %g -> 0x%04x -> %g\n", COLOR_GREEN, COLOR_RESET, orig,
             bf, back);
      rt_pass++;
    } else {
      printf("%sFAIL%s: %g -> 0x%04x -> %g\n", COLOR_RED, COLOR_RESET, orig, bf,
             back);
    }
  }

  printf("Round-trip passed %d/%d.\n", rt_pass, num_rt);
  printf("float_to_bf16 test completed\n\n");
}

void test_fmv(void) {
  printf("Testing fmv:\n");
  int cnt_fail_fmv_x_h = 0;
  int cnt_fail_fmv_h_x = 0;
  for (uint16_t i = 0; i < 0xFFFF; i++) {
    float_bits orig;
    orig.u = (uint32_t)i;
    float_bits back_0;
    back_0.u = fmv_x_h(orig.f);
    float_bits back_1;
    back_1.f = fmv_h_x(orig.u);
    if ((uint16_t)back_0.u != (uint16_t)orig.u) {
      cnt_fail_fmv_x_h++;
    }
    if ((uint16_t)back_1.u != (uint16_t)orig.u) {
      cnt_fail_fmv_h_x++;
    }
  }
  if (cnt_fail_fmv_x_h == 0) {
    printf("%sPASS%s: fmv_x_h passed all tests.\n", COLOR_GREEN, COLOR_RESET);
  } else {
    printf("%sFAIL%s: fmv_x_h failed %d/%d times.\n\n", COLOR_RED, COLOR_RESET,
           cnt_fail_fmv_x_h, 0xFFFF);
  }
  if (cnt_fail_fmv_h_x == 0) {
    printf("%sPASS%s: fmv_h_x passed all tests.\n\n", COLOR_GREEN, COLOR_RESET);
  } else {
    printf("%sFAIL%s: fmv_h_x failed %d/%d times.\n\n", COLOR_RED, COLOR_RESET,
           cnt_fail_fmv_h_x, 0xFFFF);
  }
}