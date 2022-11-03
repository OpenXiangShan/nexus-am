#include <amtest.h>
#include <xs.h>

#define READ_WORD(addr)        (*((volatile uint32_t *)(addr)))
#define WRITE_WORD(addr, data) (*((volatile uint32_t *)(addr)) = (data))

#define CONTEXT_M 0
#define CONTEXT_S 1
#define PLIC_PRIORITY          (PLIC_BASE_ADDR + 0x4UL)
#define PLIC_PENDING           (PLIC_BASE_ADDR + 0x1000UL)
#define PLIC_ENABLE(c)         (PLIC_BASE_ADDR + 0x2000UL + c*0x80UL)
#define PLIC_THRESHOLD(c)      (PLIC_BASE_ADDR + 0x200000UL + c*0x1000UL)
#define PLIC_CLAIM(c)          (PLIC_BASE_ADDR + 0x200004UL + c*0x1000UL)
// External interrupts start with index PLIC_EXT_INTR_OFFSET(interrupt source 0 is not used)
#define PLIC_EXT_INTR_OFFSET 1

#define BUS_ERROR_INTERRUPT 256
#define L3_ERROR_INTERRUPT 257

// set `TEST_L3` to test l3 cache error 
// unset `TEST_L3` to test flow of BEU
#define TEST_L3

// BEU constants
#define BEU_BASE 0x1f10010000UL
#define BEU_ENABLE_REG (BEU_BASE + 0x10UL)
#define BEU_PLIC_INTERRUPT_REG (BEU_BASE + 0x18UL)

// error handle
#define error(msg) {printf(msg); _halt(1);}

static volatile uint32_t should_claim = -1;
static volatile bool should_trigger = false;
static volatile int current_context = CONTEXT_S;

#ifndef TEST_L3
static void enable_plic_bus_error_interrupt() {
  // enable bus error interrupt 0
  plic_enable(CONTEXT_S, BUS_ERROR_INTERRUPT + PLIC_EXT_INTR_OFFSET);
}

static void disable_plic_bus_error_interrupt() {
  // disable bus error interrupt 0
  plic_disable(CONTEXT_S, BUS_ERROR_INTERRUPT + PLIC_EXT_INTR_OFFSET);
}

static void config_BEU() {
  // enable icache , dcache , l2 cache ecc; 7 means 0b111
  // 0b1(l2 cache ecc)1(dcache ecc)1(icache ecc)
  WRITE_WORD(BEU_ENABLE_REG, READ_WORD(BEU_ENABLE_REG) | 7);
  WRITE_WORD(BEU_PLIC_INTERRUPT_REG, READ_WORD(BEU_PLIC_INTERRUPT_REG) | 7);
}
#else
static void enable_plic_l3_cache_error_interrupt() {
  // enable l3 cache error interrupt
  plic_enable(CONTEXT_S, L3_ERROR_INTERRUPT + PLIC_EXT_INTR_OFFSET);
}

static void disable_plic_l3_cache_error_interrupt() {
  // disable l3 cache error interrupt
  plic_disable(CONTEXT_S, L3_ERROR_INTERRUPT + PLIC_EXT_INTR_OFFSET);
}
#endif

void handle_ext_intr() {
  if (!should_trigger) {
    error("should not trigger\n");
  }
  uint32_t claim = plic_get_claim(current_context);  // READ_WORD(PLIC_CLAIM(current_context));
  printf("an interrupt is detected, plic claim is: %d\n", claim);
  if (claim) {
    if (claim != should_claim) {
      error("ERROR: is the external interrupt bit in PLIC cleared correctly?\n");
    }
    plic_clear_intr(claim); // CLEAR_INTR(claim - PLIC_EXT_INTR_OFFSET);
    plic_clear_claim(current_context, claim); // WRITE_WORD(PLIC_CLAIM(current_context), claim);
    // NOTE: claim will always be 257 as beu always sends ecc error to plic
    should_claim = -1;
    // disable ecc error interrupt to continue
    #ifndef TEST_L3
      disable_plic_bus_error_interrupt();
    #else
      disable_plic_l3_cache_error_interrupt();
    #endif
  }
  else {
    error("ERROR: no claim?\n");
  }
}

_Context *handle_external_trap(_Event ev, _Context *ctx) {
  switch(ev.event) {
    case _EVENT_IRQ_TIMER:
      printf("t"); break;
    case _EVENT_IRQ_IODEV:
      printf("d"); handle_ext_intr(); break;
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

static inline void __attribute__((optimize("O0"))) wait_time(int cnt) {
  char blocks[20][512];
  while(cnt--) {
    for(int i = 0; i < 20; i++) {
      blocks[i][0] = 1;
      blocks[i][1] = blocks[i][0];
    }
  }
}

void test_BEU() {

  // NOTE: we are under S mode during testing 
  // enable supervisor external interrupts
  asm volatile("csrs sie, %0" : : "r"((1 << 9)));
  asm volatile("csrs sstatus, 2");
  plic_intr_init();

  #ifndef TEST_L3
    // config BEU to enable cache ecc error interrupt
    config_BEU();

    should_trigger = true;
    should_claim = BUS_ERROR_INTERRUPT + PLIC_EXT_INTR_OFFSET;
    // enable plic source of BEU
    enable_plic_bus_error_interrupt();

    // we expect an interrupt from now on
    wait_time(100);
  #else 
    // test l3 cache error
    should_trigger = true;
    should_claim = L3_ERROR_INTERRUPT + PLIC_EXT_INTR_OFFSET;
    // enable plic source of l3 cache error
    enable_plic_l3_cache_error_interrupt();

    // we expect an interrupt from now on
    wait_time(100);
  #endif

  if(should_claim != -1) {
    #ifndef TEST_L3
      error("beu interrupt is not triggered or not handled correctly\n");
    #else
      error("l3 cache error interrupt is not triggered or not handled correctly\n");
    #endif
  }

  printf("beu test passed!!!\n");
  _halt(0);
}
