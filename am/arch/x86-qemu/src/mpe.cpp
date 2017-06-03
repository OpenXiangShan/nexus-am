#include <am.h>
#include <x86.h>
#include <am-x86.h>

extern "C" {
  int current_cpu();
}

#define MP_PROC  0x00

#define MP_MAGIC 0x5f504d5f // _MP_

struct MPConf {     // configuration table header
  u8 signature[4];       // "PCMP"
  u16 length;        // total table length
  u8 version;        // [14]
  u8 checksum;         // all bytes must add up to 0
  u8 product[20];      // product id
  u32 *oemtable;         // OEM table pointer
  u16 oemlength;       // OEM table length
  u16 entry;         // entry count
  u32 *lapicaddr;        // address of local APIC
  u16 xlength;         // extended table length
  u8 xchecksum;        // extended table checksum
  u8 reserved;
};
typedef struct MPConf MPConf;

struct MPDesc {
  int magic;
  MPConf *conf; // MP config table addr
  u8 length; // 1
  u8 specrev; // [14]
  u8 checksum; // all bytes add to 0
  u8 type;   // config type
  u8 imcrp;
  u8 reserved[3];
};
typedef struct MPDesc MPDesc;

int _NR_CPU = 0;
extern u32 *lapic;

static MPDesc *search() {
  for (char *st = (char*)0xf0000; st != (char*)0xffffff; st ++) {
    if (*(u32*)st == MP_MAGIC) { // starts with magic _MP_
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
        _NR_CPU ++;
        break;
      }
      default:
        p += 8;
    }
  }
}

static void (* volatile _entry)();

static ulong ap_boot = 0;

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

  for (int cpu = 1; cpu < _NR_CPU; cpu ++) {
    *(u16*)(0x7c00 + 510) = 0x55aa;
    *reinterpret_cast<u32*>(0x7000) = 0x007c00ea;  // code for ljmp
    *reinterpret_cast<u32*>(0x7004) = 0x00000000;
    *(void**)(0x7010) = (void*)mp_entry;
    *reinterpret_cast<u32*>(0x7020) += 1024; // 1KB bootstrap stack
    lapic_bootap(cpu, 0x7000);
    while (_atomic_xchg(&ap_boot, 0) != 1);
  }

  mp_entry();
}

ulong _atomic_xchg(volatile ulong *addr, ulong newval) {
  ulong result;
  asm volatile("lock; xchgl %0, %1":
    "+m"(*addr), "=a"(result): "1"(newval): "cc");
  return result;
}

void _barrier() {
  asm volatile("":::"memory");
}

