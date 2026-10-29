#include <klib.h>
#include <stdint.h>

/* The assembly ABI stores raw FPR bits and accrued exception flags. */
typedef struct {
  uint64_t bits;
  uint32_t flags;
  uint32_t reserved;
} Result;

#define NODES 256
#define PAGE_WORDS 512
#define BOX32(x) (UINT64_C(0xffffffff00000000) | UINT64_C(x))

extern void functional_cases(Result *out);
#define SINGLE(name) extern void name(Result *out)
SINGLE(add_chain);
SINGLE(mul_chain);
SINGLE(fma_multiplicand_chain);
SINGLE(fma_rs2_chain);
SINGLE(fma_accumulator_chain);
SINGLE(mul_to_fma_acc);
SINGLE(duplicate_source_wakeup);
SINGLE(late_arrival);
SINGLE(fma_to_store);
SINGLE(fma_to_convert);
#define MEMORY(name) extern void name(Result *out, const uint64_t *input)
MEMORY(load_hot_addend);
MEMORY(load_hot_multiplicand);
MEMORY(load_cold_addend);
MEMORY(load_cold_multiplicand);
MEMORY(fma_cascade);
MEMORY(fma_pressure);
MEMORY(branch_squash);

static Result results[NODES + 8];
/* The image carries initialized inputs; cold pages are untouched before use. */
extern const uint64_t hot_addend[NODES], hot_multiplicand[NODES];
extern const uint64_t cold_addend[NODES][PAGE_WORDS];
extern const uint64_t cold_multiplicand[NODES][PAGE_WORDS];
extern const uint64_t cascade_input[NODES][PAGE_WORDS];
extern const uint64_t pressure_input[NODES][PAGE_WORDS];
extern const uint64_t squash_input[NODES][PAGE_WORDS];
static volatile uint64_t warm_sink;

/* Exact conversion of a nonnegative integer <= 2^52 using integer operations. */
static uint64_t integer_double(uint64_t value) {
  if (!value) return 0;
  unsigned exponent = 0;
  for (uint64_t v = value; v > 1; v >>= 1) ++exponent;
  return ((uint64_t)(1023 + exponent) << 52) |
         ((value << (52 - exponent)) & UINT64_C(0x000fffffffffffff));
}

static int check(const char *name, unsigned index, uint64_t expected,
                 uint32_t expected_flags) {
  Result actual = results[index];
  if (actual.bits == expected && actual.flags == expected_flags) return 0;
  printf("FMA_WAKEUP_FAIL case=%s index=%u actual=0x%08x%08x "
         "expected=0x%08x%08x flags=0x%x expected_flags=0x%x\n",
         name, index, (unsigned)(actual.bits >> 32), (unsigned)actual.bits,
         (unsigned)(expected >> 32), (unsigned)expected, actual.flags,
         expected_flags);
  return 1;
}

static int run_functional(void) {
  static const char *const names[] = {
    "s.fmadd", "s.fmsub", "s.fnmsub", "s.fnmadd",
    "s.rd_rs1", "s.rd_rs2", "s.rd_rs3",
    "s.producer_rs1", "s.producer_rs2", "s.producer_rs3",
    "s.producer_all_sources", "s.round_once",
    "d.fmadd", "d.fmsub", "d.fnmsub", "d.fnmadd",
    "d.rd_rs1", "d.rd_rs2", "d.rd_rs3",
    "d.producer_rs1", "d.producer_rs2", "d.producer_rs3",
    "d.producer_all_sources", "d.round_once"
  };
  static const uint64_t expected[] = {
    BOX32(0x40e00000), BOX32(0x40a00000),
    BOX32(0xc0a00000), BOX32(0xc0e00000),
    BOX32(0x40e00000), BOX32(0x40e00000), BOX32(0x40e00000),
    BOX32(0x40e00000), BOX32(0x40e00000), BOX32(0x40e00000),
    BOX32(0x40c00000), BOX32(0xa8800000),
    UINT64_C(0x401c000000000000), UINT64_C(0x4014000000000000),
    UINT64_C(0xc014000000000000), UINT64_C(0xc01c000000000000),
    UINT64_C(0x401c000000000000), UINT64_C(0x401c000000000000),
    UINT64_C(0x401c000000000000), UINT64_C(0x401c000000000000),
    UINT64_C(0x401c000000000000), UINT64_C(0x401c000000000000),
    UINT64_C(0x4018000000000000), UINT64_C(0xb970000000000000)
  };
  functional_cases(results);
  for (unsigned i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i)
    if (check(names[i], i, expected[i], 0)) return 1;
  return 0;
}

static int run_wakeup(void) {
  struct Timing {
    const char *name;
    void (*kernel)(Result *);
    uint64_t expected;
  };
  const struct Timing cases[] = {
    {"add_chain", add_chain, integer_double(65)},
    {"mul_chain", mul_chain, UINT64_C(0x43f0000000000000)}, /* 2^64 */
    {"fma_multiplicand_chain", fma_multiplicand_chain, integer_double(65)},
    {"fma_rs2_chain", fma_rs2_chain, integer_double(65)},
    {"fma_accumulator_chain", fma_accumulator_chain, integer_double(65)},
    {"mul_to_fma_acc", mul_to_fma_acc, integer_double(UINT64_C(8589934591))},
    {"duplicate_source_wakeup", duplicate_source_wakeup,
     integer_double((UINT64_C(1) << 48) + (UINT64_C(1) << 24))},
    {"late_arrival", late_arrival, integer_double(13)},
    {"fma_to_store", fma_to_store, integer_double(7)},
    {"fma_to_convert", fma_to_convert, 7}
  };
  for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
    cases[i].kernel(results);
    if (check(cases[i].name, 0, cases[i].expected, 0)) return 1;
  }
  return 0;
}

static int run_memory_case(const char *name,
                           void (*kernel)(Result *, const uint64_t *),
                           const uint64_t *input, unsigned multiplier,
                           unsigned offset) {
  kernel(results, input);
  for (unsigned i = 0; i < NODES; ++i)
    if (check(name, i, integer_double(multiplier * (i + 1) + offset), 0))
      return 1;
  return 0;
}

static int run_recovery(void) {
  for (unsigned i = 0; i < NODES; ++i)
    warm_sink ^= ((const volatile uint64_t *)hot_addend)[i];
  if (run_memory_case("load_hot_addend", load_hot_addend, hot_addend, 1, 6))
    return 1;
  for (unsigned i = 0; i < NODES; ++i)
    warm_sink ^= ((const volatile uint64_t *)hot_multiplicand)[i];
  if (run_memory_case("load_hot_multiplicand", load_hot_multiplicand,
                      hot_multiplicand, 3, 2)) return 1;
  if (run_memory_case("load_cold_addend", load_cold_addend,
                      &cold_addend[0][0], 1, 6)) return 1;
  if (run_memory_case("load_cold_multiplicand", load_cold_multiplicand,
                      &cold_multiplicand[0][0], 3, 2)) return 1;
  if (run_memory_case("fma_cascade", fma_cascade,
                      &cascade_input[0][0], 2, 13)) return 1;
  if (run_memory_case("fma_pressure", fma_pressure,
                      &pressure_input[0][0], 1, 6)) return 1;
  for (unsigned i = NODES; i < NODES + 8; ++i)
    if (check("fma_pressure.independent", i, integer_double(NODES + 1), 0))
      return 1;
  branch_squash(results, &squash_input[0][0]);
  for (unsigned i = 0; i < NODES; ++i)
    if (check("branch_squash", i, integer_double(7), 0)) return 1;
  return 0;
}

int main(const char *args) {
  int all = !args || !*args || !strcmp(args, "all");
  int functional = all || !strcmp(args, "functional");
  int wakeup = all || !strcmp(args, "wakeup");
  int recovery = all || !strcmp(args, "recovery");
  if (!functional && !wakeup && !recovery) {
    printf("FMA_WAKEUP_FAIL case=selector actual=%s expected=all flags=0 "
           "expected_flags=0\n", args);
    return 1;
  }
  if ((functional && run_functional()) || (wakeup && run_wakeup()) ||
      (recovery && run_recovery())) return 1;
  printf("FMA_WAKEUP_PASS\n");
  return 0;
}
