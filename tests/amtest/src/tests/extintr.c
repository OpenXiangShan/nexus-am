#include <amtest.h>
#include <xs.h>

#define READ_WORD(addr)        (*((volatile uint32_t *)(addr)))
#define WRITE_WORD(addr, data) (*((volatile uint32_t *)(addr)) = (data))
#define EXTRACT_BIT(data, i)   ((data) & (0x1UL << (i)))
#define SET_BIT(data, i)       ((data) | (0x1UL << (i)))
#define CLEAR_BIT(data, i)     ((data) ^ EXTRACT_BIT(data, i))

#define INTR_REG_WIDTH         32
#define INTR_REG_ADDR(i)       ((INTR_GEN_ADDR) + ((i) << 2))
#define INTR_REG_INDEX(i)      INTR_REG_ADDR(((i) / INTR_REG_WIDTH))
#define INTR_REG_OFFSET(i)     ((i) % INTR_REG_WIDTH)

#define INTR_RANDOM_ADDR(i)    ((INTR_RANDOM) + ((i) << 2))

#define READ_INTR_REG(i)  READ_WORD(INTR_REG_ADDR(i))
#define READ_INTR(i)     EXTRACT_BIT(READ_INTR_REG(INTR_REG_INDEX(i)), INTR_REG_OFFSET(i))
#define CLEAR_INTR(i)    WRITE_WORD(INTR_REG_INDEX(i), CLEAR_BIT(READ_INTR_REG(INTR_REG_INDEX(i)), INTR_REG_OFFSET(i)))
#define SET_INTR(i)      WRITE_WORD(INTR_REG_INDEX(i), SET_BIT(READ_INTR_REG(INTR_REG_INDEX(i)), INTR_REG_OFFSET(i)))

#define CONTEXT_M 0
#define CONTEXT_S 1
#define PLIC_PRIORITY          (PLIC_BASE_ADDR + 0x4UL)
#define PLIC_PENDING           (PLIC_BASE_ADDR + 0x1000UL)
#define PLIC_ENABLE(c)         (PLIC_BASE_ADDR + 0x2000UL + c*0x80UL)
#define PLIC_THRESHOLD(c)      (PLIC_BASE_ADDR + 0x200000UL + c*0x1000UL)
#define PLIC_CLAIM(c)          (PLIC_BASE_ADDR + 0x200004UL + c*0x1000UL)
// External interrupts start with index PLIC_EXT_INTR_OFFSET
#define PLIC_EXT_INTR_OFFSET   1

// CSR.MIE
#define MEIE 11
#define SEIE 9
#define MSIE 3
#define SSIE 1

static volatile uint32_t should_claim = -1;
static volatile bool random_claim = false;
static volatile bool should_trigger = false;
static volatile int current_context = CONTEXT_M;
static volatile uint32_t claim_count[MAX_EXTERNAL_INTR + MAX_INTERNAL_INTR] = {0};
static volatile uint32_t claim_count_all = 0;

void s2m() {
  // simply write to mie to trigger an illegal instruction exception
  // m mode will set mie to enable meip
  asm volatile("csrs mie, 0");
}

void do_wfi() {
  asm volatile("wfi");
}

// void m2s() {
//   asm volatile("csrw mepc, ra; li ra, MSTATUS_MPP; csrc mstatus, ra;");
//   asm volatile("li ra, MSTATUS_MPP&(MSTATUS_MPP>>1); csrs mstatus, ra; mret");
// }

#if defined(__ARCH_RISCV64_XS_SOUTHLAKE) || defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
  const uint32_t MAX_RAND_ITER = 2;
#else
  const uint32_t MAX_RAND_ITER = 50;
#endif

void do_ext_intr() {
  if (!should_trigger) {
    printf("should not trigger\n");
    _halt(2);
  }
  uint32_t claim = plic_get_claim(current_context);  // READ_WORD(PLIC_CLAIM(current_context));
  // printf("DO_EXT_INTR: claim %d should_claim %d\n", claim, should_claim);
  if (claim) {
    if (!random_claim && claim != should_claim) {
      printf("ERROR: is the external interrupt bit in PLIC cleared correctly?\n");
      assert(0);
    }
    claim_count[claim]++;
    claim_count_all++;
    plic_clear_intr(claim); // CLEAR_INTR(claim - PLIC_EXT_INTR_OFFSET);
    plic_clear_claim(current_context, claim); // WRITE_WORD(PLIC_CLAIM(current_context), claim);
    if (!random_claim && plic_get_claim(current_context)/*READ_WORD(PLIC_CLAIM(current_context))*/ != 0) {
      printf("ERROR: do you clear the external interrupt source correctly?\n");
      assert(0);
    }
    should_claim = -1;
    // simply write to mie to trigger an illegal instruction exception
    // m mode will set mie to enable meip
    asm volatile("csrs mie, 0");
  }
  else {
    printf("ERROR: no claim?\n");
    _halt(1);
  }
}

_Context *external_trap(_Event ev, _Context *ctx) {
  switch(ev.event) {
    case _EVENT_IRQ_TIMER:
      printf("t"); break;
    case _EVENT_IRQ_IODEV:
      printf("d"); do_ext_intr(); break;
    case _EVENT_YIELD:
      printf("y"); break;
    default:
      printf("u"); _halt(1);
  }
  return ctx;
}

static void plic_intr_init() {
  for (int i = 0; i < MAX_EXTERNAL_INTR + MAX_INTERNAL_INTR + PLIC_EXT_INTR_OFFSET; i++) {
    // WRITE_WORD(PLIC_PRIORITY + i * sizeof(uint32_t), 0x1);
    plic_set_priority(i, 0x1);
  }
  for (int i = 0; i < MAX_EXTERNAL_INTR + PLIC_EXT_INTR_OFFSET + MAX_INTERNAL_INTR; i += 32) {
    plic_disable_word(CONTEXT_M, i); // WRITE_WORD(PLIC_ENABLE(CONTEXT_M) + i/8, 0);
    plic_disable_word(CONTEXT_S, i); // WRITE_WORD(PLIC_ENABLE(CONTEXT_S) + i/8, 0);
  }
  plic_set_threshold(CONTEXT_M, 0x0); // WRITE_WORD(PLIC_THRESHOLD(CONTEXT_M), 0x0);
  plic_set_threshold(CONTEXT_S, 0x0); // WRITE_WORD(PLIC_THRESHOLD(CONTEXT_S), 0x0);
}

void external_trigger(bool shall_trigger, bool wfi, int context) {
  printf("should trigger: %s\n", shall_trigger ? "Yes" : "No");
  current_context = context;
  should_trigger = shall_trigger;

  int origin_claim;
  for (int i = 0; i < MAX_RAND_ITER; i++) {
    should_claim = (rand() % MAX_EXTERNAL_INTR) + PLIC_EXT_INTR_OFFSET;
    origin_claim = should_claim;
    // printf("interation:%d should_claim %d, setting intr\n", i, should_claim);
    plic_enable(current_context, should_claim); // WRITE_WORD(PLIC_ENABLE(current_context) + (should_claim / 32) * 4, (1UL << (should_claim % 32)));
    plic_set_intr(should_claim); // SET_INTR(should_claim - PLIC_EXT_INTR_OFFSET);
    if (shall_trigger && wfi) {
      do_wfi();
    }
    else {
      int counter = 0;
      while (should_claim != -1 && counter < 2000) {
        counter++;
      }
    }
    if (should_trigger) {
      if (should_claim != -1) {
        printf("external interrupt %d is not triggered!\n", should_claim);
        _halt(1);
      }
    } else {
      // clear
      uint32_t claim =  plic_get_claim(current_context); // READ_WORD(PLIC_CLAIM(current_context));
      plic_clear_intr(claim); // CLEAR_INTR(claim - PLIC_EXT_INTR_OFFSET);
      plic_clear_claim(current_context, claim); // WRITE_WORD(PLIC_CLAIM(current_context), claim);
    }

    plic_disable(current_context, origin_claim); // WRITE_WORD(PLIC_ENABLE(current_context) + (origin_claim / 32) * 4, 0);
  }
  printf("current test finishes\n");
}

void random_trigger() {
  should_trigger = true;
  current_context = CONTEXT_S;
  random_claim = true;
  claim_count_all = 0;
  WRITE_WORD(INTR_RANDOM_MASK, 0xfff);
  for (int i = 0; i < (MAX_EXTERNAL_INTR + 31) / 32; i++) {
    WRITE_WORD(INTR_RANDOM_ADDR(i), 0xffffffff);
  }
  // Add one here because PLIC interrupt number starts at 1.
  for (int i = 0; i < (MAX_EXTERNAL_INTR + 1 + 31) / 32; i++) {
    WRITE_WORD(PLIC_ENABLE(CONTEXT_S) + i * 4, 0xffffffff);
  }
  void hello_intr_n(int n);
  hello_intr_n(10);
  while (claim_count_all < MAX_RAND_ITER);
  printf("random test finishes\n");
}

void external_intr() {

  /** four stage test:
    * 1. m intr at m mode | mie | sie -> should trigger
    * 2. m intr at s mode -> should trigger
    * 3. s intr at m mode -> should not trigger
    * 4. s intr at s mode -> should trigger
    * but the state is s mode
    */
  // enable supervisor external interrupts
  asm volatile("csrs sie, %0" : : "r"((1 << 9)));
  asm volatile("csrs sstatus, 2");
  plic_intr_init();
  // enable WFI instruction
  asm volatile("csrs 0x5c4, %0" : : "r"((0x7)));

  // trigger interrupts
  // s-mode
  printf("s-mode & external interrupt from context-s(1)\n");
  external_trigger(true, false, CONTEXT_S);
  printf("s-mode & external interrupt from context-s(1), with wfi\n");
  external_trigger(true, true, CONTEXT_S);
  printf("s-mode & external interrupt from context-m(0)\n");
  external_trigger(true, false, CONTEXT_M);
  printf("s-mode & external interrupt from context-m(0), with wfi\n");
  external_trigger(true, true, CONTEXT_M);

  // M-mode external inter is not finishec
  // s2m(); // turn to m-mode
  // asm volatile("csrs mie, %0" : : "r"(1L << 11));
  // asm volatile("csrs mstatus, %0" : : "r"(1 << 3));
  // // m-mode
  // external_trigger(true, CONTEXT_M);
  // external_trigger(false, CONTEXT_S);

#if !defined(__ARCH_RISCV64_XS_SOUTHLAKE) && !defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
  plic_intr_init();
  random_trigger();
#endif

  // for (int i = 0; i < MAX_EXTERNAL_INTR + MAX_INTERNAL_INTR; i++) {
  //   printf("claim_count[%d] = %lu\n", i, claim_count[i]);
  // }
  printf("external interrupt test passed!!!\n");
}
