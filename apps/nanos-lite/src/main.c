#include "common.h"

void init_mm(void);
void init_device(void);
void init_irq(void);
void init_fs(void);
void load_prog(const char *);

int main() {
  init_mm();

  Log("'Hello World!' from Nanos-lite");
  Log("Build time: %s, %s", __TIME__, __DATE__);

  extern uint8_t ramdisk_start;
  extern uint8_t ramdisk_end;
  Log("ramdisk info: start = %p, end = %p, size = %d bytes",
      &ramdisk_start, &ramdisk_end, &ramdisk_end - &ramdisk_start);

  init_device();

  Log("Initializing interrupt/exception handler...");
  init_irq();

  init_fs();

  load_prog("/bin/pal");
  load_prog("/bin/hello");

  _trap();

  panic("Should not reach here");
}
