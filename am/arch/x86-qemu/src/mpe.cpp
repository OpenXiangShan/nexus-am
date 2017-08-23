#include <am.h>
#include <x86.h>
#include <am-x86.h>

extern "C" {
  int current_cpu();
}

#define MP_PROC  0x00

#define MP_MAGIC 0x5f504d5f // _MP_

struct MPConf {     // configuration table header
  uint8_t signature[4];       // "PCMP"
  uint16_t length;        // total table length
  uint8_t version;        // [14]
  uint8_t checksum;         // all bytes must add up to 0
  uint8_t product[20];      // product id
  uint32_t *oemtable;         // OEM table pointer
  uint16_t oemlength;       // OEM table length
  uint16_t entry;         // entry count
  uint32_t *lapicaddr;        // address of local APIC
  uint16_t xlength;         // extended table length
  uint8_t xchecksum;        // extended table checksum
  uint8_t reserved;
};
typedef struct MPConf MPConf;

struct MPDesc {
  int magic;
  MPConf *conf; // MP config table addr
  uint8_t length; // 1
  uint8_t specrev; // [14]
  uint8_t checksum; // all bytes add to 0
  uint8_t type;   // config type
  uint8_t imcrp;
  uint8_t reserved[3];
};
typedef struct MPDesc MPDesc;

int numcpu = 0;
extern uint32_t *lapic;

static MPDesc *search() {
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
  MPDesc *mp = search();
  MPConf *conf = mp->conf;
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
}

int _ncpu() {
  return numcpu;
}

static void (* volatile _entry)();

static intptr_t ap_boot = 0;

static void mp_entry() {
  if (_cpu() != 0) {
    lapic_init();
    ioapic_enable(IRQ_KBD, _cpu());
  }
  _atomic_xchg(&ap_boot, 1);
  _entry();
}

void _mpe_init(void (*entry)()) {
  _entry = entry;

  for (int cpu = 1; cpu < numcpu; cpu ++) {
    *(uint16_t*)(0x7c00 + 510) = 0x55aa;
    *reinterpret_cast<uint32_t*>(0x7000) = 0x007c00ea;  // code for ljmp
    *reinterpret_cast<uint32_t*>(0x7004) = 0x00000000;
    *(void**)(0x7010) = (void*)mp_entry;
    *reinterpret_cast<uint32_t*>(0x7020) += 1024; // 1KB bootstrap stack
    lapic_bootap(cpu, 0x7000);
    while (_atomic_xchg(&ap_boot, 0) != 1);
  }

  mp_entry();
}

intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval) {
  intptr_t result;
  asm volatile("lock; xchgl %0, %1":
    "+m"(*addr), "=a"(result): "1"(newval): "cc");
  return result;
}

/*
void _barrier() {
  asm volatile("":::"memory");
}
*/
