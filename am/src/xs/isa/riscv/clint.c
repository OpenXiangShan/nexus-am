#include <xs.h>
#include <nemu.h>

typedef struct {
    uintptr_t mtimecmp;
    uintptr_t time_inc;
    uintptr_t temp[3];
} ClintInfo;

/* mscratch is per hart, so its timer state and compare register must match. */
static ClintInfo timer_handles[MAX_CPU];

#if defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV64_XS) || defined(__ARCH_RISCV64_XS_DUAL) || defined(__ARCH_RISCV64_XS_SOUTHLAKE) || defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
#define CLINT_MMIO (RTC_ADDR - 0xbff8)
#define TIME_INC 0x800
#else
#define CLINT_MMIO 0xa2000000
#define TIME_INC 0x800
#endif
#define CLINT_MTIMECMP (CLINT_MMIO + 0x4000)
#define CLINT_MTIMECMP_STRIDE sizeof(uint64_t)

static ClintInfo *current_timer_handle(void) {
    int cpu = _cpu();
    assert(cpu >= 0 && cpu < MAX_CPU);
    return &timer_handles[cpu];
}

/*
 * Note that timer interrupt is always triggered under machine mode
 * Machine mode interrupt handler redirects timer interrupt to supervisor
 * intr test in amtest should be able to detect CLINT timer interrupt
 * i.e. you may use intr test to check if CLINT works
 */

/*
 * set timer increase value
 */
void set_timer_inc(uintptr_t inc) {
    ClintInfo *timer_handle = current_timer_handle();
    timer_handle->time_inc = inc;
}

/*
 * timer initialize
 * set interrupt handler
 */
void init_timer() {
    ClintInfo *timer_handle = current_timer_handle();
    timer_handle->mtimecmp = CLINT_MTIMECMP +
                              (uintptr_t)_cpu() * CLINT_MTIMECMP_STRIDE;
    /* Keep a period selected before CTE changes the privilege level. */
    if (timer_handle->time_inc == 0) {
        timer_handle->time_inc = TIME_INC;
    }
    *(volatile uint64_t *)(timer_handle->mtimecmp) =
        *(volatile uint64_t *)(RTC_ADDR) + timer_handle->time_inc;
    asm volatile("csrw mscratch, %0" : : "r"(timer_handle));
}
/*
 * enable machine mode timer interrupt
 */
void enable_timer() {
  // set machine timer interrupt
  asm volatile("csrs mie, %0" : : "r"((1 << 7) | (1 << 5) | (1 << 1)));
  asm volatile("csrs mstatus, %0" : : "r"(1 << 3));
}

/*
 * disable machine mode timer interrupt
 */
void disable_timer() {
  // unset machine timer interrupt
  asm volatile("csrc mie, %0" : : "r"((1 << 7) | (1 << 5) | (1 << 1)));
}
