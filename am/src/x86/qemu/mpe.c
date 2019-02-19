#include "../x86-qemu.h"

int ncpu = 0;
struct cpu_local cpuinfo[MAX_CPU];
static void (* volatile user_entry)();
static volatile intptr_t apboot_done = 0;
volatile struct boot_info *bootrec = (void *)0x7000;

static void percpu_entry();
static void ap_entry();
static void jump_to(void (*entry)());

int _mpe_init(void (*entry)()) {
  user_entry = entry;
  jump_to(percpu_entry);
  panic("bug: should not return");
  return 1;
}

int _ncpu() {
  return ncpu;
}

int _cpu(void) {
  return lapic[8] >> 24;
}

intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval) {
  intptr_t result;
  asm volatile ("lock xchgl %0, %1":
    "+m"(*addr), "=a"(result) : "1"(newval) : "cc");
  return result;
}

static void percpu_entry() {
  if (_cpu() == 0) { // bootstrap cpu, boot all aps
    for (int cpu = 1; cpu < ncpu; cpu++) {
      bootrec->is_ap = 1;
      bootrec->entry = percpu_entry;
      lapic_bootap(cpu, 0x7c00);
      while (_atomic_xchg(&apboot_done, 0) != 1) {
        pause();
      }
    }
    user_entry();
  } else { // this is an ap
    jump_to(ap_entry);
  }
}

static void ap_entry() {
  percpu_initgdt();
  percpu_initirq();
  percpu_initlapic();
  percpu_initpg();
  _atomic_xchg(&apboot_done, 1);
  user_entry();
}

static void jump_to(void (*entry)()) {
  void *esp = CPU->stack + sizeof(CPU->stack);
  asm volatile (
    "movl %0, %%esp;" // switch stack, and the bootstrap stack at
    "call *%1"        // 0x7000 can be reused by ap's bootloader
      : : "r"(esp) , "r"(entry));
}
