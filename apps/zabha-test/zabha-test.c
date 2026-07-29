#include <am.h>
#include <klib.h>
#include <xsextra.h>

/*
 * This test uses Zabha/Zacas mnemonics directly, so it requires a RISC-V
 * assembler new enough to recognize them, such as the GCC 15 nightly toolchain.
 */
#define DEFINE_AMO(width_name, op_name, mnemonic)                            \
  static intptr_t amo_##op_name##_##width_name(volatile void *addr,          \
                                                uintptr_t value) {           \
    intptr_t old;                                                            \
    asm volatile(                                                            \
      mnemonic " %0, %2, (%1)"                                               \
      : "=&r"(old)                                                            \
      : "r"(addr), "r"(value)                                                \
      : "memory");                                                            \
    return old;                                                              \
  }

#define DEFINE_AMO_PAIR(op_name, mnemonic) \
  DEFINE_AMO(b, op_name, mnemonic ".b")    \
  DEFINE_AMO(h, op_name, mnemonic ".h")

#define DEFINE_AMO_ORDERED_PAIR(op_name, mnemonic, order) \
  DEFINE_AMO(b, op_name, mnemonic ".b." order)            \
  DEFINE_AMO(h, op_name, mnemonic ".h." order)

DEFINE_AMO_PAIR(add,   "amoadd")
DEFINE_AMO_PAIR(swap,  "amoswap")
DEFINE_AMO_PAIR(xor,   "amoxor")
DEFINE_AMO_PAIR(or,    "amoor")
DEFINE_AMO_PAIR(and,   "amoand")
DEFINE_AMO_PAIR(min,   "amomin")
DEFINE_AMO_PAIR(max,   "amomax")
DEFINE_AMO_PAIR(minu,  "amominu")
DEFINE_AMO_PAIR(maxu,  "amomaxu")

/* Ordering variants use AMOADD as the representative Zabha operation. */
DEFINE_AMO_ORDERED_PAIR(add_rl,   "amoadd",  "rl")
DEFINE_AMO_ORDERED_PAIR(add_aq,   "amoadd",  "aq")
DEFINE_AMO_ORDERED_PAIR(add_aqrl, "amoadd",  "aqrl")
DEFINE_AMO_ORDERED_PAIR(swap_rl,  "amoswap", "rl")

static void amo_add_b_discard(volatile void *addr, uintptr_t value) {
  asm volatile(
    "amoadd.b x0, %1, (%0)"
    :
    : "r"(addr), "r"(value)
    : "memory");
}

static void amo_add_h_discard(volatile void *addr, uintptr_t value) {
  asm volatile(
    "amoadd.h x0, %1, (%0)"
    :
    : "r"(addr), "r"(value)
    : "memory");
}

static intptr_t amo_swap_b_zero(volatile void *addr) {
  intptr_t old;
  asm volatile(
    "amoswap.b %0, x0, (%1)"
    : "=&r"(old)
    : "r"(addr)
    : "memory");
  return old;
}

static intptr_t amo_swap_h_zero(volatile void *addr) {
  intptr_t old;
  asm volatile(
    "amoswap.h %0, x0, (%1)"
    : "=&r"(old)
    : "r"(addr)
    : "memory");
  return old;
}

#ifdef ZABHA_WITH_ZACAS
#define DEFINE_AMOCAS(width_name, order_name, mnemonic)                      \
  static intptr_t amocas_##order_name##_##width_name(                        \
      volatile void *addr, intptr_t expected, uintptr_t desired) {           \
    asm volatile(                                                            \
      mnemonic " %0, %2, (%1)"                                               \
      : "+&r"(expected)                                                        \
      : "r"(addr), "r"(desired)                                              \
      : "memory");                                                            \
    return expected;                                                         \
  }

#define DEFINE_AMOCAS_PAIR(order_name, mnemonic) \
  DEFINE_AMOCAS(b, order_name, mnemonic ".b")    \
  DEFINE_AMOCAS(h, order_name, mnemonic ".h")

#define DEFINE_AMOCAS_ORDERED_PAIR(order_name, mnemonic, order) \
  DEFINE_AMOCAS(b, order_name, mnemonic ".b." order)            \
  DEFINE_AMOCAS(h, order_name, mnemonic ".h." order)

DEFINE_AMOCAS_PAIR(base, "amocas")
DEFINE_AMOCAS_ORDERED_PAIR(rl,   "amocas", "rl")
DEFINE_AMOCAS_ORDERED_PAIR(aq,   "amocas", "aq")
DEFINE_AMOCAS_ORDERED_PAIR(aqrl, "amocas", "aqrl")

static void amocas_b_rd_zero(volatile void *addr, uintptr_t desired) {
  asm volatile(
    "amocas.b x0, %1, (%0)"
    :
    : "r"(addr), "r"(desired)
    : "memory");
}

static void amocas_h_rd_zero(volatile void *addr, uintptr_t desired) {
  asm volatile(
    "amocas.h x0, %1, (%0)"
    :
    : "r"(addr), "r"(desired)
    : "memory");
}

static intptr_t amocas_b_rs2_zero(volatile void *addr, intptr_t expected) {
  asm volatile(
    "amocas.b %0, x0, (%1)"
    : "+&r"(expected)
    : "r"(addr)
    : "memory");
  return expected;
}

static intptr_t amocas_h_rs2_zero(volatile void *addr, intptr_t expected) {
  asm volatile(
    "amocas.h %0, x0, (%1)"
    : "+&r"(expected)
    : "r"(addr)
    : "memory");
  return expected;
}
#endif

typedef intptr_t (*amo_fn_t)(volatile void *, uintptr_t);

enum amo_kind {
  OP_ADD,
  OP_SWAP,
  OP_XOR,
  OP_OR,
  OP_AND,
  OP_MIN,
  OP_MAX,
  OP_MINU,
  OP_MAXU,
};

struct amo_op {
  const char *name;
  enum amo_kind kind;
  amo_fn_t byte;
  amo_fn_t half;
};

static const struct amo_op amo_ops[] = {
  { "amoadd",  OP_ADD,  amo_add_b,  amo_add_h  },
  { "amoswap", OP_SWAP, amo_swap_b, amo_swap_h },
  { "amoxor",  OP_XOR,  amo_xor_b,  amo_xor_h  },
  { "amoor",   OP_OR,   amo_or_b,   amo_or_h   },
  { "amoand",  OP_AND,  amo_and_b,  amo_and_h  },
  { "amomin",  OP_MIN,  amo_min_b,  amo_min_h  },
  { "amomax",  OP_MAX,  amo_max_b,  amo_max_h  },
  { "amominu", OP_MINU, amo_minu_b, amo_minu_h },
  { "amomaxu", OP_MAXU, amo_maxu_b, amo_maxu_h },
};

static const uint16_t byte_old[] = {
  0x00, 0x01, 0x7f, 0x80, 0xfe, 0xff, 0x55, 0xaa,
};
static const uint16_t byte_src[] = {
  0xff, 0x02, 0x80, 0x7f, 0x02, 0x01, 0xaa, 0x55,
};
static const uint16_t half_old[] = {
  0x0000, 0x0001, 0x7fff, 0x8000,
  0xfffe, 0xffff, 0x5555, 0xaaaa,
};
static const uint16_t half_src[] = {
  0xffff, 0x0002, 0x8000, 0x7fff,
  0x0002, 0x0001, 0xaaaa, 0x5555,
};

typedef union __attribute__((aligned(8))) {
  uint64_t word;
  uint8_t byte[8];
  uint16_t half[4];
} amo_cell_t;

static int checks;
static int failures;
static volatile uintptr_t alignment_exception_cause;

static void check_value(const char *group, const char *op, int width,
                        int lane, int vector, uintptr_t actual,
                        uintptr_t expected) {
  checks++;
  if (actual == expected) {
    return;
  }

  failures++;
  if (failures <= 32) {
    printf("FAIL %-8s %-10s width=%d lane=%d vector=%d "
           "actual=0x%lx expected=0x%lx\n",
           group, op, width, lane, vector,
           (unsigned long)actual, (unsigned long)expected);
  }
}

static uint32_t width_mask(int width) {
  return width == 1 ? 0xffu : 0xffffu;
}

static uintptr_t sign_extend(uint32_t value, int width) {
  if (width == 1) {
    return (uintptr_t)(intptr_t)(int8_t)value;
  }
  return (uintptr_t)(intptr_t)(int16_t)value;
}

static uint32_t reference_result(enum amo_kind kind, int width,
                                 uint32_t old, uint32_t src) {
  uint32_t mask = width_mask(width);
  int32_t signed_old = width == 1 ? (int8_t)old : (int16_t)old;
  int32_t signed_src = width == 1 ? (int8_t)src : (int16_t)src;

  old &= mask;
  src &= mask;
  switch (kind) {
    case OP_ADD:  return (old + src) & mask;
    case OP_SWAP: return src;
    case OP_XOR:  return old ^ src;
    case OP_OR:   return old | src;
    case OP_AND:  return old & src;
    case OP_MIN:  return signed_old < signed_src ? old : src;
    case OP_MAX:  return signed_old > signed_src ? old : src;
    case OP_MINU: return old < src ? old : src;
    case OP_MAXU: return old > src ? old : src;
  }
  return 0;
}

static uint64_t replace_lane(uint64_t word, int width, int lane,
                             uint32_t value) {
  int shift = lane * width * 8;
  uint64_t mask = (uint64_t)width_mask(width) << shift;
  return (word & ~mask) | (((uint64_t)value << shift) & mask);
}

static volatile void *cell_lane(volatile amo_cell_t *cell, int width,
                                int lane) {
  if (width == 1) {
    return &cell->byte[lane];
  }
  return &cell->half[lane];
}

static void test_one_width(const struct amo_op *op, int width) {
  const uint16_t *old_values = width == 1 ? byte_old : half_old;
  const uint16_t *src_values = width == 1 ? byte_src : half_src;
  int lanes = width == 1 ? 8 : 4;
  int vectors = (int)(sizeof(byte_old) / sizeof(byte_old[0]));
  amo_fn_t fn = width == 1 ? op->byte : op->half;
  const uint64_t background = 0x0123456789abcdefULL;

  for (int lane = 0; lane < lanes; lane++) {
    for (int vector = 0; vector < vectors; vector++) {
      volatile amo_cell_t cell;
      uint32_t old = old_values[vector];
      uint32_t src = src_values[vector];
      uintptr_t source_with_ignored_bits =
        (uintptr_t)0xa55adead00000000ULL | src;
      uint32_t result = reference_result(op->kind, width, old, src);
      uint64_t initial = replace_lane(background, width, lane, old);
      uint64_t expected_word = replace_lane(initial, width, lane, result);

      cell.word = initial;
      intptr_t returned = fn(cell_lane(&cell, width, lane),
                             source_with_ignored_bits);

      check_value("return", op->name, width, lane, vector,
                  (uintptr_t)returned, sign_extend(old, width));
      check_value("memory", op->name, width, lane, vector,
                  (uintptr_t)cell.word, (uintptr_t)expected_word);
    }
  }
}

static void test_all_amo_operations(void) {
  int op_count = (int)(sizeof(amo_ops) / sizeof(amo_ops[0]));
  for (int i = 0; i < op_count; i++) {
    test_one_width(&amo_ops[i], 1);
    test_one_width(&amo_ops[i], 2);
  }
}

struct ordering_case {
  const char *name;
  enum amo_kind kind;
  amo_fn_t byte;
  amo_fn_t half;
};

static const struct ordering_case ordering_cases[] = {
  { "amoadd.rl",   OP_ADD,  amo_add_rl_b,   amo_add_rl_h   },
  { "amoadd.aq",   OP_ADD,  amo_add_aq_b,   amo_add_aq_h   },
  { "amoadd.aqrl", OP_ADD,  amo_add_aqrl_b, amo_add_aqrl_h },
  { "amoswap.rl",  OP_SWAP, amo_swap_rl_b,  amo_swap_rl_h  },
};

static void test_ordering_encodings(void) {
  const uint64_t background = 0xfedcba9876543210ULL;
  int count = (int)(sizeof(ordering_cases) / sizeof(ordering_cases[0]));

  for (int i = 0; i < count; i++) {
    for (int width = 1; width <= 2; width++) {
      volatile amo_cell_t cell;
      int lane = width == 1 ? 5 : 2;
      uint32_t old = width == 1 ? 0x80 : 0x8000;
      uint32_t src = width == 1 ? 0x7f : 0x7fff;
      uint32_t result = reference_result(ordering_cases[i].kind, width,
                                         old, src);
      uint64_t initial = replace_lane(background, width, lane, old);
      uint64_t expected_word = replace_lane(initial, width, lane, result);
      amo_fn_t fn = width == 1 ? ordering_cases[i].byte
                               : ordering_cases[i].half;

      cell.word = initial;
      intptr_t returned = fn(cell_lane(&cell, width, lane), src);
      check_value("ordering", ordering_cases[i].name, width, lane, 0,
                  (uintptr_t)returned, sign_extend(old, width));
      check_value("memory", ordering_cases[i].name, width, lane, 0,
                  (uintptr_t)cell.word, (uintptr_t)expected_word);
    }
  }
}

static void test_x0_register_cases(void) {
  const uint64_t background = 0x8877665544332211ULL;
  volatile amo_cell_t cell;
  uint64_t expected;
  intptr_t returned;

  cell.word = replace_lane(background, 1, 3, 0xfe);
  expected = replace_lane(cell.word, 1, 3, 0x03);
  amo_add_b_discard(&cell.byte[3], 0x05);
  check_value("rd=x0", "amoadd", 1, 3, 0,
              (uintptr_t)cell.word, (uintptr_t)expected);

  cell.word = replace_lane(background, 2, 2, 0xfffe);
  expected = replace_lane(cell.word, 2, 2, 0x0003);
  amo_add_h_discard(&cell.half[2], 0x0005);
  check_value("rd=x0", "amoadd", 2, 2, 0,
              (uintptr_t)cell.word, (uintptr_t)expected);

  cell.word = replace_lane(background, 1, 6, 0x80);
  expected = replace_lane(cell.word, 1, 6, 0);
  returned = amo_swap_b_zero(&cell.byte[6]);
  check_value("rs2=x0", "amoswap", 1, 6, 0,
              (uintptr_t)returned, sign_extend(0x80, 1));
  check_value("memory", "amoswap", 1, 6, 0,
              (uintptr_t)cell.word, (uintptr_t)expected);

  cell.word = replace_lane(background, 2, 1, 0x8000);
  expected = replace_lane(cell.word, 2, 1, 0);
  returned = amo_swap_h_zero(&cell.half[1]);
  check_value("rs2=x0", "amoswap", 2, 1, 0,
              (uintptr_t)returned, sign_extend(0x8000, 2));
  check_value("memory", "amoswap", 2, 1, 0,
              (uintptr_t)cell.word, (uintptr_t)expected);
}

_Context *alignment_exception_handler(_Event event, _Context *ctx) {
  (void)event;
  // ctx->scause = 6;	STORE/AMO addr unalign
  alignment_exception_cause = ctx->scause;
  ctx->sepc += 4;
  return ctx;
}

void test_halfword_alignment_exception(void) {
  volatile amo_cell_t cell;
  const uint64_t initial = 0x0123456789abcdefULL;

  _cte_init(alignment_exception_handler);

  cell.word = initial;
  alignment_exception_cause = 0;
  (void)amo_add_h(&cell.byte[1], 1);

  check_value("exception", "amoadd.h", 2, 1, 0,
              alignment_exception_cause == 6,
              1);
  check_value("memory", "amoadd.h", 2, 1, 0,
              (uintptr_t)cell.word, (uintptr_t)initial);
}

#ifdef ZABHA_WITH_ZACAS
typedef intptr_t (*amocas_fn_t)(volatile void *, intptr_t, uintptr_t);

struct amocas_ordering_case {
  const char *name;
  amocas_fn_t byte;
  amocas_fn_t half;
};

static const struct amocas_ordering_case amocas_ordering_cases[] = {
  { "amocas",      amocas_base_b, amocas_base_h },
  { "amocas.rl",   amocas_rl_b,   amocas_rl_h   },
  { "amocas.aq",   amocas_aq_b,   amocas_aq_h   },
  { "amocas.aqrl", amocas_aqrl_b, amocas_aqrl_h },
};

static void test_one_amocas_width(int width) {
  const uint64_t background = 0x1032547698badcfeULL;
  int lanes = width == 1 ? 8 : 4;
  int count = (int)(sizeof(amocas_ordering_cases) /
                    sizeof(amocas_ordering_cases[0]));

  for (int i = 0; i < count; i++) {
    amocas_fn_t fn = width == 1 ? amocas_ordering_cases[i].byte
                                : amocas_ordering_cases[i].half;
    for (int lane = 0; lane < lanes; lane++) {
      volatile amo_cell_t cell;
      uint32_t old = width == 1 ? 0x80u : 0x8000u;
      uint32_t desired = width == 1 ? 0x5au : 0x5aa5u;
      uint32_t mismatch = old ^ 1u;
      uintptr_t ignored = (uintptr_t)0xc33cdead00000000ULL;
      uint64_t initial = replace_lane(background, width, lane, old);
      uint64_t success_word = replace_lane(initial, width, lane, desired);

      cell.word = initial;
      intptr_t returned = fn(cell_lane(&cell, width, lane),
                             (intptr_t)(ignored | old), ignored | desired);
      check_value("cas-pass", amocas_ordering_cases[i].name,
                  width, lane, 0, (uintptr_t)returned,
                  sign_extend(old, width));
      check_value("memory", amocas_ordering_cases[i].name,
                  width, lane, 0, (uintptr_t)cell.word,
                  (uintptr_t)success_word);

      cell.word = initial;
      returned = fn(cell_lane(&cell, width, lane),
                    (intptr_t)(ignored | mismatch), ignored | desired);
      check_value("cas-fail", amocas_ordering_cases[i].name,
                  width, lane, 1, (uintptr_t)returned,
                  sign_extend(old, width));
      check_value("memory", amocas_ordering_cases[i].name,
                  width, lane, 1, (uintptr_t)cell.word,
                  (uintptr_t)initial);
    }
  }
}

static void test_amocas_x0_cases(void) {
  const uint64_t background = 0xa1b2c3d4e5f60718ULL;
  volatile amo_cell_t cell;
  uint64_t expected;
  intptr_t returned;

  cell.word = replace_lane(background, 1, 2, 0);
  expected = replace_lane(cell.word, 1, 2, 0x5a);
  amocas_b_rd_zero(&cell.byte[2], 0x5a);
  check_value("cas-rd=x0", "amocas", 1, 2, 0,
              (uintptr_t)cell.word, (uintptr_t)expected);

  cell.word = replace_lane(background, 2, 2, 0);
  expected = replace_lane(cell.word, 2, 2, 0x5aa5);
  amocas_h_rd_zero(&cell.half[2], 0x5aa5);
  check_value("cas-rd=x0", "amocas", 2, 2, 0,
              (uintptr_t)cell.word, (uintptr_t)expected);

  cell.word = replace_lane(background, 1, 4, 0x80);
  expected = replace_lane(cell.word, 1, 4, 0);
  returned = amocas_b_rs2_zero(&cell.byte[4], 0x80);
  check_value("cas-rs2=x0", "amocas", 1, 4, 0,
              (uintptr_t)returned, sign_extend(0x80, 1));
  check_value("memory", "amocas", 1, 4, 0,
              (uintptr_t)cell.word, (uintptr_t)expected);

  cell.word = replace_lane(background, 2, 1, 0x8000);
  expected = replace_lane(cell.word, 2, 1, 0);
  returned = amocas_h_rs2_zero(&cell.half[1], 0x8000);
  check_value("cas-rs2=x0", "amocas", 2, 1, 0,
              (uintptr_t)returned, sign_extend(0x8000, 2));
  check_value("memory", "amocas", 2, 1, 0,
              (uintptr_t)cell.word, (uintptr_t)expected);
}

static void test_amocas_operations(void) {
  test_one_amocas_width(1);
  test_one_amocas_width(2);
  test_amocas_x0_cases();
}
#endif

#ifdef ZABHA_SMP
#define BYTE_INCREMENTS  100
#define HALF_INCREMENTS 1000
#define MESSAGE_ROUNDS   64

static volatile amo_cell_t shared_byte __attribute__((aligned(64)));
static volatile amo_cell_t shared_half __attribute__((aligned(64)));
static volatile amo_cell_t message_flag __attribute__((aligned(64)));
static volatile uint64_t message_payload __attribute__((aligned(64)));
static volatile int smp_failures[2];

static void run_smp_tests(void) {
  int cpu = _cpu();

  if (cpu == 0) {
    shared_byte.word = 0;
    shared_half.word = 0;
    message_flag.word = 0;
    message_payload = 0;
    smp_failures[0] = 0;
    smp_failures[1] = 0;
  }
  _barrier();

  for (int i = 0; i < BYTE_INCREMENTS; i++) {
    amo_add_b(&shared_byte.byte[3], 1);
  }
  for (int i = 0; i < HALF_INCREMENTS; i++) {
    amo_add_h(&shared_half.half[2], 1);
  }
  _barrier();

  if (cpu == 0) {
    if (shared_byte.byte[3] != (uint8_t)(2 * BYTE_INCREMENTS)) {
      smp_failures[0]++;
    }
    if (shared_half.half[2] != (uint16_t)(2 * HALF_INCREMENTS)) {
      smp_failures[0]++;
    }
  }
  _barrier();

  for (int round = 1; round <= MESSAGE_ROUNDS; round++) {
    if (cpu == 0) {
      message_payload = 0x5a5a000000000000ULL | (uint64_t)round;
      amo_swap_rl_b(&message_flag.byte[0], 1);
      while (amo_add_aq_b(&message_flag.byte[0], 0) != 0) {
      }
    } else {
      while (amo_add_aq_b(&message_flag.byte[0], 0) != 1) {
      }
      if (message_payload !=
          (0x5a5a000000000000ULL | (uint64_t)round)) {
        smp_failures[1]++;
      }
      amo_swap_rl_b(&message_flag.byte[0], 0);
    }
  }
  _barrier();

  if (cpu == 0) {
    check_value("smp", "amoadd.b", 1, 3, 0,
                shared_byte.byte[3], (uint8_t)(2 * BYTE_INCREMENTS));
    check_value("smp", "amoadd.h", 2, 2, 0,
                shared_half.half[2], (uint16_t)(2 * HALF_INCREMENTS));
    check_value("smp", "aq/rl", 1, 0, 0,
                (uintptr_t)(smp_failures[0] + smp_failures[1]), 0);
  }
  _barrier();
}
#endif

static void run_functional_tests(void) {
  test_all_amo_operations();
  test_ordering_encodings();
  test_x0_register_cases();
  test_halfword_alignment_exception();
#ifdef ZABHA_WITH_ZACAS
  test_amocas_operations();
#endif
}

int main(void) {
#ifdef ZABHA_SMP
  _mpe_setncpu('2');
  if (_cpu() == 0) {
    _mpe_wakeup(1);
    printf("Zabha test: RV64, dual-core mode\n");
    run_functional_tests();
  }
  _barrier();
  run_smp_tests();
  if (_cpu() != 0) {
    while (1) {
      asm volatile("wfi");
    }
  }
#else
  printf("Zabha test: RV64, single-core mode\n");
  run_functional_tests();
#endif

#ifdef ZABHA_WITH_ZACAS
  printf("Zacas-dependent AMOCAS.B/H: enabled\n");
#else
  printf("Zacas-dependent AMOCAS.B/H: skipped (use WITH_ZACAS=1)\n");
#endif
  printf("Zabha test summary: %d checks, %d failures\n", checks, failures);
  if (failures == 0) {
    printf("Zabha test PASS\n");
  } else if (failures > 32) {
    printf("Additional failures suppressed: %d\n", failures - 32);
  }
  return failures == 0 ? 0 : 1;
}
