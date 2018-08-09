#include <am-x86.h>

#define STACK_SZ   4096 // each processor's stack
#define MP_PROC    0x00
#define MP_MAGIC   0x5f504d5f // _MP_

int numcpu = 0;
extern uint32_t *lapic;

static MPDesc *mp_search() {
  for (char *st = (char*)0xf0000; st != (char*)0xffffff; st ++) {
    if (*(uint32_t*)st == MP_MAGIC) { // starts with magic _MP_
      MPDesc *mp = (MPDesc*)st;
      char checksum = 0;
      for (char *p = st; p != st + sizeof(MPDesc); p ++) {
        checksum += *p;
      }
      if (checksum == 0) { // checksums zero
        return mp;
      }
    }
  }
  return 0;
}

void smp_init() {
  MPConf *conf = mp_search()->conf;
  lapic = conf->lapicaddr;

  for (char *p = (char*)(conf + 1); p < (char*)conf + conf->length; ) {
    switch (*p) {
      case MP_PROC: {
        p += 20; // CPU desc has 20 bytes
        numcpu ++;
        break;
      }
      default:
        p += 8;
    }
  }
  if (numcpu > MAX_CPU) {
    numcpu = MAX_CPU;
  }
}

int _ncpu() {
  return numcpu;
}

static void (* volatile _entry)();

static intptr_t ap_boot = 0;

// TODO: use stack in heap memory, not in the low memory

static void mp_entry() {
  if (_cpu() != 0) {
    cpu_initgdt();
    lapic_init();
    ioapic_enable(IRQ_KBD, _cpu());
    cpu_initpte();
  }
  _atomic_xchg(&ap_boot, 1);
  _entry();
}

int _mpe_init(void (*entry)()) {
  _entry = entry;

  for (int cpu = 1; cpu < numcpu; cpu ++) {
    *(uint16_t*)(0x7c00 + 510) = 0x55aa;
    *(uint32_t*)(0x7000) = 0x007c00ea;  // code for ljmp
    *(uint32_t*)(0x7004) = 0x00000000;
    *(void**)(0x7010) = (void*)mp_entry;
    *(uint32_t*)(0x7020) += 1024; // 1KB bootstrap stack
    lapic_bootap(cpu, 0x7000);
    while (_atomic_xchg(&ap_boot, 0) != 1);
  }

  mp_entry();
  return 0; // never reaches here
}

intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval) {
  intptr_t result;
  asm volatile("lock; xchgl %0, %1":
    "+m"(*addr), "=a"(result): "1"(newval): "cc");
  return result;
}
