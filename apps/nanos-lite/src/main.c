#include "common.h"

uint32_t loader(_Protect *);
void init_mm(void);
void init_irq(void);

#ifndef __PAGE
static inline void relocate_myself() {
  Log("Relocating myself...");
  const uintptr_t new_start = 0x800000;
  extern char _start, _end, rel_finish;
  // FIXME: in x86-qemu, _start may not be the start of the text segment
  void *start = &_start;
  void *end = &_end;
  memcpy((void *)new_start, start, end - start);
  asm volatile("jmp *%0;\n\t rel_finish:" : :"r"(&rel_finish - &_start + new_start));
}
#endif

int main() {
  Log("'Hello World!' from Nanos-lite");
  Log("Build time: %s, %s", __TIME__, __DATE__);

  extern uint8_t ramdisk_start;
  extern uint8_t ramdisk_end;
  Log("ramdisk info: start = %p, end = %p, size = %d bytes",
      &ramdisk_start, &ramdisk_end, &ramdisk_end - &ramdisk_start);

  static uint32_t entry;

#ifdef __PAGE

  Log("Initializing memory manager...");
  init_mm();

  Log("Initializing interrupt/exception handler...");
  init_irq();

  _Protect user_as;  // user process address space

  _protect(&user_as);

  Log("Loading user program...");
  entry = loader(&user_as);

  Log("Switching to user address space...");
  _switch(&user_as);

  /* Set the %esp for user program */
  asm volatile("movl %0, %%esp" : : "i"(0xc0000000));

#else

  relocate_myself();

  entry = loader(NULL);

#endif

  /* Keep the `bt' command happy. */
  asm volatile("movl $0, %ebp");
  asm volatile("subl $16, %esp");

  ((void(*)(void))entry)();

  _halt(0);
}
