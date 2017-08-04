#include "common.h"

void init_device(void);
void init_irq(void);
void load_first_prog(void);

int main() {
  Log("'Hello World!' from Nanos-lite");
  Log("Build time: %s, %s", __TIME__, __DATE__);

  extern uint8_t ramdisk_start;
  extern uint8_t ramdisk_end;
  Log("ramdisk info: start = %p, end = %p, size = %d bytes",
      &ramdisk_start, &ramdisk_end, &ramdisk_end - &ramdisk_start);

  init_device();

  Log("Initializing interrupt/exception handler...");
  init_irq();

  load_first_prog();

  _trap();

  panic("Should not reach here");
}
