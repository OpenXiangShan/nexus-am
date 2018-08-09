#include <x86.h>
#include <am.h>
#include <am-x86.h>

_Area _heap; // the heap memory defined in AM spec

int main();
static void memory_init();

// the bootloader jumps here,
// with a (small) bootstrap stack
void _start() {
  smp_init();
  lapic_init();
  ioapic_init();
  memory_init();
  cpu_initgdt();

  int ret = main();
  _halt(ret);
}

#define SERIAL_PORT 0x3f8

void _putc(char ch) {
  while ((inb(SERIAL_PORT + 5) & 0x20) == 0);
  outb(SERIAL_PORT, ch);
}

void _halt(int code) {
  char buf[] = "Exited (#).\n";
  buf[8] = '0' + code;
  puts(buf);
  asm volatile("cli; hlt");
  while(1);
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
  _heap.start = (void*)st;
  _heap.end   = (void*)ed;
}


#define MP_PROC    0x00
#define MP_MAGIC   0x5f504d5f // _MP_

static MPDesc *mp_search() {
  for (char *st = (char*)0xf0000; st != (char*)0xffffff; st ++) {
    if (*(uint32_t*)st == MP_MAGIC)
      return (MPDesc*)st;
  }
  panic("_MP_ not found");
  return NULL;
}

void smp_init() {
  MPConf *conf = mp_search()->conf;
  lapic = conf->lapicaddr;

  for (char *p = (char*)(conf + 1); p < (char*)conf + conf->length; ) {
    if (*p == MP_PROC) {
      p += 20; ncpu ++;
    } else {
      p += 8;
    }
  }
  if (ncpu > MAX_CPU) {
    panic("cannot support > MAX_CPU processors");
  }
}

