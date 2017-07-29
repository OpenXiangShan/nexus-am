#include "common.h"
#include "memory.h"
#include "fs.h"

#define INIT_FILE "/bin/pal"

void ramdisk_read(void *, off_t, size_t);

#define STACK_SIZE (128 * PGSIZE)

uintptr_t loader() {
  Log("Loading user program %s...", INIT_FILE);

  extern uint8_t ramdisk_start;
  extern uint8_t ramdisk_end;
  ramdisk_read((void *)0x4000000, 0, &ramdisk_end - &ramdisk_start);

  return 0x4000000;
}
