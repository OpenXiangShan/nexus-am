#include <x86.h>
#include <am.h>
#include <am-x86.h>

_Area _heap; // the heap memory defined in AM spec

int main();
static void memory_init();
static void sys_init();

// the bootloader jumps here,
//   with a (small) bootstrap stack
void _start() {
  // setup a C runtime environment
  sys_init();
  memory_init();
  percpu_initgdt();
  percpu_initlapic();
  ioapic_init();

  int ret = main();
  _halt(ret);
}

#define COM1_PORT 0x3f8

void _putc(char ch) {
  // no protection, only works for qemu
  outb(COM1_PORT, ch);
}

void _halt(int code) {
  cli();
  allcpu_halt();
  char buf[] = "Exited (#).\n";
  buf[8] = '0' + code;
  puts(buf);
  thiscpu_die();
}

#define MP_PROC    0x00
#define MP_MAGIC   0x5f504d5f // _MP_

static void sys_init() {
  for (char *st = (char *)0xf0000; st != (char *)0xffffff; st ++) {
    if (*(volatile uint32_t *)st == MP_MAGIC) {
      volatile MPConf *conf = ((volatile MPDesc *)st)->conf;
      lapic = conf->lapicaddr;
      for (volatile char *ptr = (char *)(conf + 1);
                 ptr < (char *)conf + conf->length; ) {
        if (*ptr == MP_PROC) {
          ptr += 20;
          if (++ncpu > MAX_CPU) {
            panic("cannot support > MAX_CPU processors");
          }
        } else {
          ptr += 8;
        }
      }
      return;
    }
  }
  panic("seems not an x86-qemu machine");
}

static void memory_init() {
  extern char end;
  uintptr_t st, ed, step = 1L << 20; // probe step: 1 MB
  st = ed = (((uintptr_t)&end) & ~(step - 1)) + step;
  while (1) {
    volatile uint32_t *ptr = (uint32_t*)ed;
    *ptr = 0x5a5a5a5a; // write then read
    if (*ptr == 0x5a5a5a5a) ed += step; // check passed, memory is okay
    else break; // hit the end of the physical memory
  }
  _heap = RANGE(st, ed);
}
