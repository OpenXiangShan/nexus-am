#include "common.h"

_Protect* get_user_as(void);
uint32_t loader(_Protect *);
void init_mm(void);
void init_irq(void);

int main() {
  Log("'Hello World!' from Nanos-lite");
  Log("Build time: %s, %s", __TIME__, __DATE__);

  extern uint8_t ramdisk_start;
  extern uint8_t ramdisk_end;
  Log("ramdisk info: start = %p, end = %p, size = %d bytes",
      &ramdisk_start, &ramdisk_end, &ramdisk_end - &ramdisk_start);

  _ioe_init();

  static uint32_t entry;

#ifdef __PAGE

  Log("Initializing memory manager...");
  init_mm();

  Log("Initializing interrupt/exception handler...");
  init_irq();

  _Protect *p = get_user_as();  // user process address space

  _protect(p);

  entry = loader(p);

  Log("Switching to user address space...");
  _switch(p);

  /* Set the %esp for user program */
  asm volatile("movl %0, %%esp" : : "i"(0xc0000000));

#else

  entry = loader(NULL);

#endif

  /* Keep the `bt' command happy. */
  asm volatile("movl $0, %ebp");
  asm volatile("subl $16, %esp");

  ((void(*)(void))entry)();

  _halt(0);
}
