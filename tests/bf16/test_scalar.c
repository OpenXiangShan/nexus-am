#include "bf16.h"

// Test data for BF16 conversion
test_case_t test_cases[] = {
    // Normal numbers
    {1.0f, 0x3F80, "1.0"},
    {0.0f, 0x0000, "0.0"},
    {-0.0f, 0x8000, "-0.0"},
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
    {0x1p-133f, 0x0001, "Smallest positive BF16 subnormal"},
    {-0x1p-133f, 0x8001, "Smallest negative BF16 subnormal"},
    {0x1p-126f, 0x0080, "Smallest positive BF16 normal"},

    // Edge cases
    {0.0078125f, 0x3C00, "2^-7"},
    {0x1.01p+0f, 0x3F80, "RNE tie with even BF16 LSB"},
    {0x1.03p+0f, 0x3F82, "RNE tie with odd BF16 LSB"},
    {0x1.fep+127f, 0x7F7F, "Largest finite BF16"},
    {0x1.fffffep+127f, 0x7F80, "Largest finite FP32 rounds to +inf"},
};

int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

static uint32_t bf16_test_rand_state = 0x6D2B79F5u;

static uint32_t bf16_test_rand(void) {
  bf16_test_rand_state = bf16_test_rand_state * 1664525u + 1013904223u;
  return bf16_test_rand_state;
}

static const char *rounding_mode_name(int rm) {
  static const char *names[] = {"RNE", "RTZ", "RDN", "RUP", "RMM"};
  return (rm >= 0 && rm < 5) ? names[rm] : "UNKNOWN";
}

static void test_float_to_bf16_boundary_bits(void) {
  typedef struct {
    uint32_t input_bits;
    uint16_t expected;
    const char *desc;
  } bit_case_t;

  static const bit_case_t bit_cases[] = {
      {0x00000000u, 0x0000, "+0"},
      {0x80000000u, 0x8000, "-0"},
      {0x00008000u, 0x0000, "Subnormal tie to +0"},
      {0x00008001u, 0x0001, "Subnormal just above +0 tie"},
      {0x00018000u, 0x0002, "Subnormal tie to even non-zero"},
      {0x007FFFFFu, 0x0080, "Largest FP32 subnormal"},
      {0x00800000u, 0x0080, "Smallest FP32 normal"},
      {0x3F808000u, 0x3F80, "Normal tie to even"},
      {0x3F818000u, 0x3F82, "Normal tie away from odd"},
      {0x7F7F0000u, 0x7F7F, "Largest finite BF16 exact"},
      {0x7F7F7FFFu, 0x7F7F, "Just below overflow tie"},
      {0x7F7F8000u, 0x7F80, "Overflow tie to +inf"},
      {0xFF7F8000u, 0xFF80, "Overflow tie to -inf"},
      {0x7F800001u, 0x7FC0, "Positive signaling NaN"},
      {0xFFBFFFFFu, 0xFFC0, "Negative NaN"},
  };

  int pass_count = 0;
  int num_cases = sizeof(bit_cases) / sizeof(bit_cases[0]);

  printf("Boundary bit-pattern test (float -> bf16):\n");
  for (int i = 0; i < num_cases; i++) {
    float_bits input;
    input.u = bit_cases[i].input_bits;
    uint16_t actual = float_to_bf16(input.f);
    int ok = bf16_matches_float_conversion(input.f, actual,
                                           bit_cases[i].expected);

    if (ok) {
      pass_count++;
      printf("%sPASS%s: %s (0x%08x) -> 0x%04x\n", COLOR_GREEN, COLOR_RESET,
             bit_cases[i].desc, bit_cases[i].input_bits, actual);
    } else {
      printf("%sFAIL%s: %s (0x%08x) -> 0x%04x, expected 0x%04x\n",
             COLOR_RED, COLOR_RESET, bit_cases[i].desc,
             bit_cases[i].input_bits, actual, bit_cases[i].expected);
    }
  }

  printf("Boundary bit-pattern passed %d/%d.\n\n", pass_count, num_cases);
}

static void test_float_to_bf16_rounding_modes(void) {
  typedef struct {
    uint32_t input_bits;
    const char *desc;
  } rm_case_t;

  static const rm_case_t rm_cases[] = {
      {0x3F808000u, "+1.00390625 tie, even low BF16 LSB"},
      {0x3F818000u, "+1.01171875 tie, odd low BF16 LSB"},
      {0x3F800001u, "+1.0 plus tiny fraction"},
      {0x3F80FFFFu, "+1.0 just below next BF16"},
      {0xBF808000u, "-1.00390625 tie"},
      {0xBF800001u, "-1.0 minus tiny fraction"},
      {0xBF80FFFFu, "-1.0 just below next BF16 magnitude"},
      {0x00008000u, "+subnormal half-way to min BF16 subnormal"},
      {0x80008000u, "-subnormal half-way to min BF16 subnormal"},
      {0x7F7F7FFFu, "+max finite below overflow tie"},
      {0x7F7F8000u, "+max finite overflow tie"},
      {0xFF7F8000u, "-max finite overflow tie"},
  };

  int fail_count = 0;
  int num_cases = sizeof(rm_cases) / sizeof(rm_cases[0]);

  printf("Rounding mode test (float -> bf16):\n");
  for (int rm = 0; rm <= 4; rm++) {
    set_rounding_mode(rm);
    for (int i = 0; i < num_cases; i++) {
      float_bits input;
      input.u = rm_cases[i].input_bits;
      uint16_t expected = float_to_bf16_soft_rm(input.f, rm);
      uint16_t actual = float_to_bf16(input.f);

      if (bf16_matches_float_conversion(input.f, actual, expected)) {
        printf("%sPASS%s: [%s] %s (0x%08x) -> 0x%04x\n", COLOR_GREEN,
               COLOR_RESET, rounding_mode_name(rm), rm_cases[i].desc,
               rm_cases[i].input_bits, actual);
      } else {
        printf("%sFAIL%s: [%s] %s (0x%08x) -> 0x%04x, expected 0x%04x\n",
               COLOR_RED, COLOR_RESET, rounding_mode_name(rm),
               rm_cases[i].desc, rm_cases[i].input_bits, actual, expected);
        fail_count++;
      }
    }
  }
  set_rounding_mode(0);

  if (fail_count == 0) {
    printf("%sPASS%s: all rounding modes matched scalar oracle.\n\n",
           COLOR_GREEN, COLOR_RESET);
  } else {
    printf("%sFAIL%s: rounding mode test failed %d/%d cases.\n\n", COLOR_RED,
           COLOR_RESET, fail_count, num_cases * 5);
  }
}

static void test_float_to_bf16_random(void) {
  int fail_count = 0;
  const int num_random = 128;

  printf("Deterministic random test (float -> bf16):\n");
  for (int i = 0; i < num_random; i++) {
    float_bits input;
    input.u = bf16_test_rand();
    uint16_t expected = float_to_bf16_soft(input.f);
    uint16_t actual = float_to_bf16(input.f);

    if (!bf16_matches_float_conversion(input.f, actual, expected)) {
      if (fail_count < 8) {
        printf("%sFAIL%s: bits=0x%08x -> 0x%04x, expected 0x%04x\n",
               COLOR_RED, COLOR_RESET, input.u, actual, expected);
      }
      fail_count++;
    }
  }

  if (fail_count == 0) {
    printf("%sPASS%s: %d random FP32 patterns matched software oracle.\n\n",
           COLOR_GREEN, COLOR_RESET, num_random);
  } else {
    printf("%sFAIL%s: random float_to_bf16 failed %d/%d cases.\n\n",
           COLOR_RED, COLOR_RESET, fail_count, num_random);
  }
}

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

  set_rounding_mode(0);
  int pass_count = 0;

  for (int i = 0; i < num_test_cases; i++) {
    float f = test_cases[i].input;
    uint16_t expected = test_cases[i].expected;
    const char *desc = test_cases[i].desc;
    uint16_t bf = float_to_bf16(f);

    int ok = 0;

    ok = bf16_matches_float_conversion(f, bf, expected);

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

  test_float_to_bf16_boundary_bits();
  test_float_to_bf16_rounding_modes();
  set_rounding_mode(0);
  test_float_to_bf16_random();

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
    uint16_t mid = float_to_bf16_soft(orig);
    float back = bf16_to_float(bf);
    int ok = 0;

    if (is_nan(orig)) {
      ok = is_nan(back);
    } else if (is_inf(orig)) {
      ok = is_inf(back) && (get_sign(orig) == get_sign(back));
    } else {
      /* Normal numbers: Allow small absolute or relative errors */

      float ulp = my_fabs(orig) * (1.0f / 64.0f); // BF16 的步长 ≈ |x| / 2^7
      ok = ((my_fabs(orig - back) <= 2.0f * ulp) ||
            float_equal(orig, back, 1e-6f, 1e-5f)) &&
           (mid == bf);
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
