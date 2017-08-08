#include "common.h"

/* Uncomment these macros to enable corresponding functionality. */
#define HAS_ASYE
#define HAS_FS
#define HAS_PTE

void init_mm(void);
void init_device(void);
void init_irq(void);
void init_fs(void);
void load_prog(const char *);

int main() {
#ifdef HAS_PTE
  init_mm();
#endif

  Log("'Hello World!' from Nanos-lite");
  Log("Build time: %s, %s", __TIME__, __DATE__);

  extern uint8_t ramdisk_start;
  extern uint8_t ramdisk_end;
  Log("ramdisk info: start = %p, end = %p, size = %d bytes",
      &ramdisk_start, &ramdisk_end, &ramdisk_end - &ramdisk_start);

  init_device();

#ifdef HAS_ASYE
  Log("Initializing interrupt/exception handler...");
  init_irq();
#endif

#ifdef HAS_FS
  init_fs();
#endif

  load_prog("/bin/pal");
  load_prog("/bin/hello");
  load_prog("/bin/litenes");

  _trap();

  panic("Should not reach here");
}
