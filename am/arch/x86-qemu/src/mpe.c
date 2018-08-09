#include <am-x86.h>

int ncpu = 0;
static void (* volatile user_entry)();
static intptr_t apboot_done = 0;

static void mp_entry();
static void stack_switch(void (*entry)());

int _mpe_init(void (*entry)()) {
  user_entry = entry;
  stack_switch(mp_entry); // switch stack, and the bootstrap stack at
                          // 0x7000 can be reused by an ap's bootloader
  panic("mp_init should not return");
  return 1;
}

int _cpu(void) {
  return lapic[8] >> 24;
}

int _ncpu() {
  return ncpu;
}

intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval) {
  intptr_t result;
  asm volatile("lock xchgl %0, %1":
    "+m"(*addr), "=a"(result): "1"(newval): "cc");
  return result;
}

struct boot_info {
  int is_ap;
  void (*entry)();
};

static void ap_entry() {
  cpu_initgdt();
  lapic_init();
  ioapic_enable(IRQ_KBD, _cpu());
  cpu_initpte();
  _atomic_xchg(&apboot_done, 1);
  user_entry();
}

static void stack_switch(void (*entry)()) {
  static uint8_t cpu_stk[MAX_CPU][4096]; // each cpu gets a 4KB stack
  asm volatile(
    "movl %0, %%esp;"
    "call *%1" : : "r"(&cpu_stk[_cpu() + 1][0]), "r"(entry));
}

static void mp_entry() { // all cpus execute mp_entry()
  if (_cpu() != 0) {
    // init an ap
    stack_switch(ap_entry);
  } else {
    // stack already swithced, boot all aps
    volatile struct boot_info *boot = (void *)0x7000;
    for (int cpu = 1; cpu < ncpu; cpu ++) {
      boot->is_ap = 1;
      boot->entry = mp_entry;
      lapic_bootap(cpu, 0x7c00);
      while (_atomic_xchg(&apboot_done, 0) != 1);
    }
    user_entry();
  }
}
