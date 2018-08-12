#include <am-x86.h>

int ncpu = 0;
static void (* volatile user_entry)();
static volatile intptr_t apboot_done = 0;

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
  __asm__ volatile ("lock xchgl %0, %1":
    "+m"(*addr), "=a"(result): "1"(newval): "cc");
  return result;
}

struct boot_info {
  int is_ap;
  void (*entry)();
};
volatile struct boot_info *boot_rec = (void *)0x7000;

static void ap_entry() {
  cpu_initgdt();
  cpu_initidt();
  cpu_initlapic();
  cpu_initpg();
  ioapic_enable(IRQ_KBD, _cpu());
  _atomic_xchg(&apboot_done, 1);
  user_entry();
}

static void stack_switch(void (*entry)()) {
  static uint8_t cpu_stk[MAX_CPU][4096]; // each cpu gets a 4KB stack
  __asm__ volatile (
    "movl %0, %%esp;"
    "call *%1" : : "r"(&cpu_stk[_cpu() + 1][0]), "r"(entry));
}

static void mp_entry() { // all cpus execute mp_entry()
  if (_cpu() != 0) {
    // init an ap
    stack_switch(ap_entry);
  } else {
    // stack already swithced, boot all aps
    for (int cpu = 1; cpu < ncpu; cpu ++) {
      boot_rec->is_ap = 1;
      boot_rec->entry = mp_entry;
      lapic_bootap(cpu, 0x7c00);
      while (_atomic_xchg(&apboot_done, 0) != 1);
    }
    user_entry();
  }
}

void cpu_die() {
  cli();
  while (1) hlt();
}

void mp_halt() {
  boot_rec->is_ap = 1;
  boot_rec->entry = cpu_die;
  for (int cpu = 0; cpu < ncpu; cpu++) {
    if (cpu != _cpu()) {
      lapic_bootap(cpu, 0x7c00);
    }
  }
}

