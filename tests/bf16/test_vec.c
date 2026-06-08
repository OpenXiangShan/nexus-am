#include "bf16.h"

static uint32_t vec_rand_state = 0x9E3779B9u;

static uint32_t vec_rand(void) {
  vec_rand_state = vec_rand_state * 1103515245u + 12345u;
  return vec_rand_state;
}

static int active_lanes(int base, int total) {
  int lanes = total - base;
  return lanes < 4 ? lanes : 4;
}

void test_vfncvtbf16() {
  printf("Testing vfncvtbf16.f.f.w (Vector convert FP32 to BF16):\n");

  // Test 1: Basic conversion
  printf("\nTest 1: Basic conversion\n");

  // Test with vector length 4
  float test_values[4] = {};
  uint16_t results[4] = {3, 3, 3, 3};

  for (int base = 0; base < num_test_cases; base += 4) {
    int lanes = active_lanes(base, num_test_cases);
    for (int i = 0; i < 4; i++) {
      test_values[i] = i < lanes ? test_cases[base + i].input : 0.0f;
      results[i] = 0xA5A5;
    }
    asm volatile("csrwi frm,0 \n"
                 "vsetivli zero, 4, e32, m1, ta, ma\n"
                 "vle32.v v2, (%0)\n"
                 "vsetivli zero, 4, e16, m1, ta, ma\n"
                 "vfncvtbf16.f.f.w v0, v2\n"
                 "vse16.v v0, (%1)\n"
                 "fence rw, rw\n"
                 :
                 : "r"(test_values), "r"(results)
                 : "v0", "v1", "v2", "memory");
    // Check results
    for (int i = 0; i < lanes; i++) {
      uint16_t expected = test_cases[base + i].expected;
      if (bf16_matches_float_conversion(test_values[i], results[i],
                                        expected)) {
        printf("%sPASS%s: %g -> 0x%04x\n", COLOR_GREEN, COLOR_RESET,
               test_values[i], results[i]);
      } else {
        printf("%sFAIL%s: %g -> 0x%04x, expected 0x%04x\n", COLOR_RED,
               COLOR_RESET, test_values[i], results[i], expected);
      }
    }
  }
  // Test 2: Masked operation
  printf("\nTest 2: Masked operation\n");

  float masked_values[4] = {test_cases[0].input, test_cases[1].input,
                            test_cases[2].input, test_cases[3].input};
  uint16_t masked_results[4] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
  uint8_t mask[4] = {1, 0, 1, 0};
  uint8_t mask_bits[] = {(mask[3] << 3) | (mask[2] << 2) | (mask[1] << 1) |
                         mask[0]};
  asm volatile("csrwi frm,0 \n"
               "vsetivli zero, 4, e16, m1, tu, mu\n"
               "vle16.v v2, (%2)\n"
               "vsetivli zero, 4, e32, m1, tu, mu\n"
               "vle32.v v4, (%0)\n"
               "vsetivli zero, 4, e8, m1, tu, mu\n"
               "vlm.v v0, (%1)\n"
               "vsetivli zero, 4, e16, m1, tu, mu\n"
               "vfncvtbf16.f.f.w v2, v4, v0.t\n"
               "vse16.v v2, (%2)\n"
               "fence rw, rw\n"
               :
               : "r"(masked_values), "r"(mask_bits), "r"(masked_results)
               : "v0", "v1", "v2", "v4", "v5", "memory");

  for (int i = 0; i < 4; i++) {
    if (mask[i]) {
      uint16_t expected = test_cases[i].expected;
      if (bf16_matches_float_conversion(masked_values[i], masked_results[i],
                                        expected)) {
        printf("%sPASS%s: [masked] %g -> 0x%04x\n", COLOR_GREEN, COLOR_RESET,
               masked_values[i], masked_results[i]);
      } else {
        printf("%sFAIL%s: [masked] %g -> 0x%04x, expected 0x%04x\n", COLOR_RED,
               COLOR_RESET, masked_values[i], masked_results[i], expected);
      }
    } else {
      if (masked_results[i] == 0xFFFF) {
        printf("%sPASS%s: [unmasked] %g -> 0x%04x (unchanged)\n", COLOR_GREEN,
               COLOR_RESET, masked_values[i], masked_results[i]);
      } else {
        printf("%sFAIL%s: [unmasked] %g -> 0x%04x, expected 0xFFFF\n",
               COLOR_RED, COLOR_RESET, masked_values[i], masked_results[i]);
      }
    }
  }

  // Test 3: Deterministic random operation
  printf("\nTest 3: Deterministic random operation\n");

  int random_fail = 0;
  for (int iter = 0; iter < 64; iter++) {
    for (int i = 0; i < 4; i++) {
      float_bits input;
      input.u = vec_rand();
      test_values[i] = input.f;
      results[i] = 0x5A5A;
    }

    asm volatile("csrwi frm,0 \n"
                 "vsetivli zero, 4, e32, m1, ta, ma\n"
                 "vle32.v v2, (%0)\n"
                 "vsetivli zero, 4, e16, m1, ta, ma\n"
                 "vfncvtbf16.f.f.w v0, v2\n"
                 "vse16.v v0, (%1)\n"
                 "fence rw, rw\n"
                 :
                 : "r"(test_values), "r"(results)
                 : "v0", "v1", "v2", "memory");

    for (int i = 0; i < 4; i++) {
      uint16_t expected = float_to_bf16_soft(test_values[i]);
      if (!bf16_matches_float_conversion(test_values[i], results[i],
                                         expected)) {
        if (random_fail < 8) {
          float_bits bits;
          bits.f = test_values[i];
          printf("%sFAIL%s: random bits=0x%08x -> 0x%04x, expected 0x%04x\n",
                 COLOR_RED, COLOR_RESET, bits.u, results[i], expected);
        }
        random_fail++;
      }
    }
  }

  if (random_fail == 0) {
    printf("%sPASS%s: vfncvtbf16 random conversion matched 256 lanes.\n",
           COLOR_GREEN, COLOR_RESET);
  } else {
    printf("%sFAIL%s: vfncvtbf16 random conversion failed %d/256 lanes.\n",
           COLOR_RED, COLOR_RESET, random_fail);
  }

  printf("\nvfncvtbf16.f.f.w test completed\n\n");
}

// Test vfwcvtbf16.f.f.v (Vector convert BF16 to FP32)
void test_vfwcvtbf16() {
  printf("Testing vfwcvtbf16.f.f.v (Vector convert BF16 to FP32):\n");

  // Test 1: Basic conversion
  printf("\nTest 1: Basic conversion\n");

  uint16_t bf16_values[4];
  float results[4];

  for (int base = 0; base < num_test_cases; base += 4) {
    int lanes = active_lanes(base, num_test_cases);
    for (int i = 0; i < 4; i++) {
      bf16_values[i] = i < lanes ? test_cases[base + i].expected : 0;
      results[i] = -3.0f;
    }
    asm volatile("csrwi frm,0 \n"
                 "vsetivli zero, 4, e16, m1, ta, ma\n"
                 "vle16.v v2, (%0)\n"
                 "vfwcvtbf16.f.f.v v0, v2\n"
                 "vsetivli zero, 4, e32, m1, ta, ma\n"
                 "vse32.v v0, (%1)\n"
                 "fence rw, rw\n"
                 :
                 : "r"(bf16_values), "r"(results)
                 : "v0", "v1", "v2", "v3",
                   "memory"); // v2-v3 for EMUL=2 with e32

    // Check results
    for (int i = 0; i < lanes; i++) {
      float_bits result_bits;
      result_bits.f = results[i];
      if (float_matches_bf16_conversion(bf16_values[i], results[i])) {
        printf("%sPASS%s: 0x%04x -> %g\n", COLOR_GREEN, COLOR_RESET,
               bf16_values[i], results[i]);
      } else {
        float expected2 = bf16_to_float_soft(bf16_values[i]);
        float_bits expected_bits;
        expected_bits.f = expected2;
        printf("%sFAIL%s: 0x%04x -> %g/%08x, expected %g/%08x\n",
               COLOR_RED, COLOR_RESET, bf16_values[i], results[i],
               result_bits.u, expected2, expected_bits.u);
      }
    }
  }

  // Test 2: Masked operation
  printf("\nTest 2: Masked operation\n");

  uint16_t masked_bf16[4] = {test_cases[0].expected, test_cases[1].expected,
                             test_cases[2].expected, test_cases[3].expected};
  float masked_results[4] = {-1.0f, -1.0f, -1.0f, -1.0f};
  uint8_t mask[4] = {1, 0, 1, 0};
  uint8_t mask_bits[] = {(mask[3] << 3) | (mask[2] << 2) | (mask[1] << 1) |
                         mask[0]};

  asm volatile("csrwi frm,0 \n"
               "vsetivli zero, 4, e32, m2, tu, mu\n"
               "vle32.v v4, (%2)\n"
               "vsetivli zero, 4, e8, m1, tu, mu\n"
               "vlm.v v0, (%1)\n"
               "vsetivli zero, 4, e16, m1, tu, mu\n"
               "vle16.v v2, (%0)\n"
               "vfwcvtbf16.f.f.v v4, v2, v0.t\n"
               "vsetivli zero, 4, e32, m1, ta, ma\n"
               "vse32.v v4, (%2)\n"
               "fence rw, rw\n"
               :
               : "r"(masked_bf16), "r"(mask_bits), "r"(masked_results)
               : "v0", "v1", "v2", "v4", "v5", "memory");

  for (int i = 0; i < 4; i++) {
    if (mask[i]) {
      if (float_matches_bf16_conversion(masked_bf16[i], masked_results[i])) {
        printf("%sPASS%s: [masked] 0x%04x -> %g\n", COLOR_GREEN, COLOR_RESET,
               masked_bf16[i], masked_results[i]);
      } else {
        float expected = bf16_to_float_soft(masked_bf16[i]);
        printf("%sFAIL%s: [masked] 0x%04x -> %g, expected %g\n", COLOR_RED,
               COLOR_RESET, masked_bf16[i], masked_results[i], expected);
      }
    } else {
      if (masked_results[i] == -1.0f) {
        printf("%sPASS%s: [unmasked] 0x%04x -> %g (unchanged)\n", COLOR_GREEN,
               COLOR_RESET, masked_bf16[i], masked_results[i]);
      } else {
        printf("%sFAIL%s: [unmasked] 0x%04x -> %g, expected -1.0\n", COLOR_RED,
               COLOR_RESET, masked_bf16[i], masked_results[i]);
      }
    }
  }

  // Test 3: Deterministic random operation
  printf("\nTest 3: Deterministic random operation\n");

  int random_fail = 0;
  for (int iter = 0; iter < 64; iter++) {
    for (int i = 0; i < 4; i++) {
      bf16_values[i] = (uint16_t)(vec_rand() >> 16);
      results[i] = -3.0f;
    }

    asm volatile("csrwi frm,0 \n"
                 "vsetivli zero, 4, e16, m1, ta, ma\n"
                 "vle16.v v2, (%0)\n"
                 "vfwcvtbf16.f.f.v v0, v2\n"
                 "vsetivli zero, 4, e32, m1, ta, ma\n"
                 "vse32.v v0, (%1)\n"
                 "fence rw, rw\n"
                 :
                 : "r"(bf16_values), "r"(results)
                 : "v0", "v1", "v2", "v3", "memory");

    for (int i = 0; i < 4; i++) {
      if (!float_matches_bf16_conversion(bf16_values[i], results[i])) {
        if (random_fail < 8) {
          float_bits actual_bits;
          float_bits expected_bits;
          actual_bits.f = results[i];
          expected_bits.f = bf16_to_float_soft(bf16_values[i]);
          printf("%sFAIL%s: random 0x%04x -> 0x%08x, expected 0x%08x\n",
                 COLOR_RED, COLOR_RESET, bf16_values[i], actual_bits.u,
                 expected_bits.u);
        }
        random_fail++;
      }
    }
  }

  if (random_fail == 0) {
    printf("%sPASS%s: vfwcvtbf16 random conversion matched 256 lanes.\n",
           COLOR_GREEN, COLOR_RESET);
  } else {
    printf("%sFAIL%s: vfwcvtbf16 random conversion failed %d/256 lanes.\n",
           COLOR_RED, COLOR_RESET, random_fail);
  }

  printf("vfwcvtbf16.f.f.v test completed\n\n");
}

// Expand vfwmaccbf16.vv instruction using vfwcvtbf16 and vfmacc
void expand_vfwmaccbf_vf(uint16_t *vs1_bf16, uint16_t *vs2_bf16,
                         float *vd_fp32) {
  asm volatile("csrwi frm,0 \n"
               "vsetivli zero, 4, e16, m1, ta, ma\n"
               "vle16.v v0, (%0)\n"
               "vfwcvtbf16.f.f.v v2, v0\n"
               "vsetivli zero, 4, e16, m1, ta, ma\n"
               "vle16.v v1, (%1)\n"
               "vfwcvtbf16.f.f.v v6, v1\n"
               "vsetivli zero, 4, e32, m1, ta, ma\n"
               "vle32.v v4, (%2)\n"
               "vfmacc.vv v4,v2,v6\n"
               "vsetivli zero, 4, e32, m1, ta, ma\n"
               "vse32.v v4, (%2)\n"
               "fence rw, rw\n"
               :
               : "r"(vs1_bf16), "r"(vs2_bf16), "r"(vd_fp32)
               : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "memory");
}

void vfwmaccbf_vf(uint16_t *vs1_bf16, uint16_t *vs2_bf16, float *vd_fp32) {
  asm volatile("csrwi frm,0 \n"
               "vsetivli zero, 4, e32, m1, ta, ma\n"
               "vle32.v v4, (%2)\n"
               "vsetivli zero, 4, e16, m1, ta, ma\n"
               "vle16.v v0, (%0)\n"
               "vle16.v v2, (%1)\n"
               "vfwmaccbf16.vv v4, v0, v2\n"
               "vsetivli zero, 4, e32, m1, ta, ma\n"
               "vse32.v v4, (%2)\n"
               "fence rw, rw\n"
               :
               : "r"(vs1_bf16), "r"(vs2_bf16), "r"(vd_fp32)
               : "v0", "v1", "v2", "v3", "v4", "v5", "memory");
}
// Test vfwmaccbf16.vv (Vector widening fused multiply-accumulate)
void test_vfwmaccbf16() {
  printf(
      "Testing vfwmaccbf16.vv (Vector widening fused multiply-accumulate):\n");

  // Test data structure for BF16 widening MACC tests
  typedef struct {
    uint16_t vs1;     // Input BF16 value 1
    uint16_t vs2;     // Input BF16 value 2
    float vd;         // Input FP32 accumulate value
    float expected;   // Expected FP32 result
    const char *desc; // Description of the test case
  } wmacc_test_case_t;

  // Test data for BF16 widening MACC
  static wmacc_test_case_t wmacc_tests[] = {
      // Normal numbers
      {0x3F80, 0x4000, 3.0f, 5.0f, "1.0 * 2.0 + 3.0"},
      {0xBF00, 0x4080, 1.0f, -1.0f, "-0.5 * 4.0 + 1.0"},
      {0x42C8, 0x3F80, 0.0f, 100.0f, "100.0 * 1.0 + 0.0"},
      {0x38D2, 0x47C3, 0.0f, 0.0001f * 100000.0f, "0.0001 * 100000.0 + 0.0"},

      // Zero cases
      {0x0000, 0x4000, 5.0f, 5.0f, "0.0 * 2.0 + 5.0"},
      {0x3F80, 0x8000, 2.0f, 2.0f, "1.0 * -0.0 + 2.0"},

      // Infinity cases
      {0x7F80, 0x3F80, 0.0f, INFINITY, "+inf * 1.0 + 0.0"},
      {0xFF80, 0x3F80, 0.0f, -INFINITY, "-inf * 1.0 + 0.0"},

      // NaN cases
      {0x7FC0, 0x3F80, 0.0f, NAN, "NaN * 1.0 + 0.0"},
  };

  int num_wmacc_tests = sizeof(wmacc_tests) / sizeof(wmacc_tests[0]);

  // Test 1: Basic operation
  printf("\nTest 1: Basic operation\n");

  uint16_t vs1_bf16[4];
  uint16_t vs2_bf16[4];
  float vd_fp32[4];
  // float results_fp32[4];

  for (int base = 0; base + 4 <= num_wmacc_tests; base += 4) {
    for (int i = base; i < base + 4; i++) {
      vs1_bf16[i - base] = wmacc_tests[i].vs1;
      vs2_bf16[i - base] = wmacc_tests[i].vs2;
      vd_fp32[i - base] = wmacc_tests[i].vd;
    }

    // expand_vfwmaccbf_vf(vs1_bf16, vs2_bf16, vd_fp32);
    vfwmaccbf_vf(vs1_bf16, vs2_bf16, vd_fp32);

    // Check results
    for (int i = base; i < base + 4; i++) {
      float expected = wmacc_tests[i].expected;
      float actual = vd_fp32[i - base];
      if (float_equal(actual, expected, 1e-6f, 1e-3f)) {
        printf("%sPASS%s: %s -> %g\n", COLOR_GREEN, COLOR_RESET,
               wmacc_tests[i].desc, actual);
      } else {
        printf("%sFAIL%s: %s -> %g, expected %g\n", COLOR_RED, COLOR_RESET,
               wmacc_tests[i].desc, actual, expected);
      }
    }
  }

  // Test 2: Masked operation
  printf("\nTest 2: Masked operation\n");
  uint16_t masked_vs1_bf16[4] = {wmacc_tests[0].vs1, wmacc_tests[1].vs1,
                                 wmacc_tests[2].vs1, wmacc_tests[3].vs1};
  uint16_t masked_vs2_bf16[4] = {wmacc_tests[0].vs2, wmacc_tests[1].vs2,
                                 wmacc_tests[2].vs2, wmacc_tests[3].vs2};
  float masked_vd_fp32[4] = {wmacc_tests[0].vd, wmacc_tests[1].vd,
                             wmacc_tests[2].vd, wmacc_tests[3].vd};
  uint8_t mask[4] = {1, 0, 1, 0};
  uint8_t mask_bits[] = {(mask[3] << 3) | (mask[2] << 2) | (mask[1] << 1) |
                         mask[0]};
  asm volatile("csrwi frm,0 \n"
               "vsetivli zero, 4, e32, m1, tu, mu\n"
               "vle32.v v4, (%2)\n"
               "vsetivli zero, 4, e8, m1, tu, mu\n"
               "vlm.v v0, (%3)\n"
               "vsetivli zero, 4, e16, m1, tu, mu\n"
               "vle16.v v6, (%0)\n"
               "vle16.v v2, (%1)\n"
               "vfwmaccbf16.vv v4, v6, v2,v0.t\n"
               "vsetivli zero, 4, e32, m1, tu, mu\n"
               "vse32.v v4, (%2)\n"
               "fence rw, rw\n"
               :
               : "r"(masked_vs1_bf16), "r"(masked_vs2_bf16),
                 "r"(masked_vd_fp32), "r"(mask_bits)
               : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "memory");
  for (int i = 0; i < 4; i++) {
    if (mask[i]) {
      float expected = wmacc_tests[i].expected;
      float actual = masked_vd_fp32[i];
      if (float_equal(actual, expected, 1e-6f, 1e-3f)) {
        printf("%sPASS%s: [masked] %s -> %g\n", COLOR_GREEN, COLOR_RESET,
               wmacc_tests[i].desc, actual);
      } else {
        printf("%sFAIL%s: [masked] %s -> %g, expected %g\n", COLOR_RED,
               COLOR_RESET, wmacc_tests[i].desc, actual, expected);
      }
    } else {
      float expected = wmacc_tests[i].vd;
      float actual = masked_vd_fp32[i];
      if (float_equal(actual, expected, 1e-6f, 1e-3f)) {
        printf("%sPASS%s: [unmasked] %s -> %g\n", COLOR_GREEN, COLOR_RESET,
               wmacc_tests[i].desc, actual);
      } else {
        printf("%sFAIL%s: [unmasked] %s -> %g, expected %g\n", COLOR_RED,
               COLOR_RESET, wmacc_tests[i].desc, actual, expected);
      }
    }
  }
  printf("\nvfwmaccbf16.vv test completed\n\n");
}

void do_vfwmaccbf16_vf(uint16_t rs1_bf16, uint16_t *vs2, float *vd) {
  asm volatile("csrwi frm,0 \n"
               "fmv.h.x ft0, %0\n"
               "vsetivli zero, 4, e32, m1, ta, ma\n"
               "vle32.v v4, (%1)\n"
               "vsetivli zero, 4, e16, m1, ta, ma\n"
               "vle16.v v2, (%2)\n"
               "vfwmaccbf16.vf v4, ft0, v2\n"
               "vsetivli zero, 4, e32, m1, ta, ma\n"
               "vse32.v v4, (%1)\n"
               "fence rw, rw\n"
               :
               : "r"((uint32_t)rs1_bf16), "r"(vd), "r"(vs2)
               : "ft0", "v2", "v3", "v4", "v5", "memory");
}

// Test vfwmaccbf16.vf (Vector-scalar widening fused multiply-accumulate)
void test_vfwmaccbf16_vf() {
  printf("Testing vfwmaccbf16.vf (Vector-scalar widening fused "
         "multiply-accumulate):\n");

  typedef struct {
    uint16_t rs1;      // Scalar BF16 operand (same for all 4 elements)
    uint16_t vs2[4];   // BF16 vector operand
    float vd[4];       // FP32 accumulator (initial)
    float expected[4]; // Expected FP32 result: vd[i] += float(rs1) *
                       // float(vs2[i])
    const char *desc;
  } vf_test_t;

  static vf_test_t vf_tests[] = {
      // rs1 = 1.0 (0x3F80)
      {0x3F80,
       {0x4000, 0xBF00, 0x0000, 0x42C8}, // vs2 = {2.0, -0.5, 0.0, 100.0}
       {3.0f, 1.0f, 5.0f, 0.0f},
       {5.0f, 0.5f, 5.0f, 100.0f},
       "rs1=1.0"},
      // rs1 = -0.5 (0xBF00)
      {0xBF00,
       {0x4080, 0x4000, 0xC000, 0x42C8}, // vs2 = {4.0, 2.0, -2.0, 100.0}
       {1.0f, 0.0f, 3.0f, 0.0f},
       {-1.0f, -1.0f, 4.0f, -50.0f},
       "rs1=-0.5"},
      // rs1 = 2.0 (0x4000)
      {0x4000,
       {0x3F80, 0xBF80, 0x4000, 0x38D2}, // vs2 = {1.0, -1.0, 2.0, ~0.0001}
       {0.0f, 0.0f, 1.0f, 0.0f},
       {2.0f, -2.0f, 5.0f, 0.0002f},
       "rs1=2.0"},
      // rs1 = +inf (0x7F80): special values
      {0x7F80,
       {0x3F80, 0xBF80, 0x4000, 0x0000}, // vs2 = {1.0, -1.0, 2.0, 0.0}
       {0.0f, 0.0f, 0.0f, 1.0f},
       {INFINITY, -INFINITY, INFINITY, NAN}, // inf*0.0+1.0 = NaN
       "rs1=+inf"},
  };

  int num_vf_tests = sizeof(vf_tests) / sizeof(vf_tests[0]);

  // Test 1: Basic operation
  printf("\nTest 1: Basic operation\n");

  for (int t = 0; t < num_vf_tests; t++) {
    float vd_copy[4];
    for (int i = 0; i < 4; i++)
      vd_copy[i] = vf_tests[t].vd[i];

    do_vfwmaccbf16_vf(vf_tests[t].rs1, (uint16_t *)vf_tests[t].vs2, vd_copy);

    for (int i = 0; i < 4; i++) {
      float expected = vf_tests[t].expected[i];
      float actual = vd_copy[i];
      if (float_equal(actual, expected, 1e-4f, 1e-3f)) {
        printf("%sPASS%s: %s * bf16(0x%04x) -> %g\n", COLOR_GREEN, COLOR_RESET,
               vf_tests[t].desc, vf_tests[t].vs2[i], actual);
      } else {
        printf("%sFAIL%s: %s * bf16(0x%04x) -> %g, expected=%g\n", COLOR_RED,
               COLOR_RESET, vf_tests[t].desc, vf_tests[t].vs2[i], actual,
               expected);
      }
    }
  }

  // Test 2: Masked operation
  printf("\nTest 2: Masked operation\n");

  float masked_vd[4];
  for (int i = 0; i < 4; i++)
    masked_vd[i] = vf_tests[0].vd[i];
  uint8_t mask[4] = {1, 0, 1, 0};
  uint8_t mask_bits[] = {(mask[3] << 3) | (mask[2] << 2) | (mask[1] << 1) |
                         mask[0]};

  asm volatile("csrwi frm,0 \n"
               "fmv.h.x ft0, %0\n"
               "vsetivli zero, 4, e32, m1, tu, mu\n"
               "vle32.v v4, (%1)\n"
               "vsetivli zero, 4, e8, m1, tu, mu\n"
               "vlm.v v0, (%3)\n"
               "vsetivli zero, 4, e16, m1, tu, mu\n"
               "vle16.v v2, (%2)\n"
               "vfwmaccbf16.vf v4, ft0, v2, v0.t\n"
               "vsetivli zero, 4, e32, m1, tu, mu\n"
               "vse32.v v4, (%1)\n"
               "fence rw, rw\n"
               :
               : "r"((uint32_t)vf_tests[0].rs1), "r"(masked_vd),
                 "r"(vf_tests[0].vs2), "r"(mask_bits)
               : "ft0", "v0", "v1", "v2", "v3", "v4", "v5", "memory");

  for (int i = 0; i < 4; i++) {
    if (mask[i]) {
      float expected = vf_tests[0].expected[i];
      float actual = masked_vd[i];
      if (float_equal(actual, expected, 1e-4f, 1e-3f)) {
        printf("%sPASS%s: [masked]  %s * bf16(0x%04x) -> %g\n", COLOR_GREEN,
               COLOR_RESET, vf_tests[0].desc, vf_tests[0].vs2[i], actual);
      } else {
        printf("%sFAIL%s: [masked]  %s * bf16(0x%04x) -> %g, "
               "expected=%g\n",
               COLOR_RED, COLOR_RESET, vf_tests[0].desc, vf_tests[0].vs2[i],
               actual, expected);
      }
    } else {
      float expected_unch = vf_tests[0].vd[i];
      float actual = masked_vd[i];
      if (float_equal(actual, expected_unch, 1e-6f, 1e-5f)) {
        printf("%sPASS%s: [unmasked] %s * bf16(0x%04x) -> %g (unchanged)\n",
               COLOR_GREEN, COLOR_RESET, vf_tests[0].desc, vf_tests[0].vs2[i],
               actual);
      } else {
        printf("%sFAIL%s: [unmasked] %s * bf16(0x%04x) -> %g, expected=%g\n",
               COLOR_RED, COLOR_RESET, vf_tests[0].desc, vf_tests[0].vs2[i],
               actual, expected_unch);
      }
    }
  }

  printf("\nvfwmaccbf16.vf test completed\n\n");
}
