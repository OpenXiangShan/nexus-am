#include "common.h"

uint32_t loader(_Protect *);
void init_mm(void);
static uint32_t entry;

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
#ifndef __PAGE
  relocate_myself();
#else

  init_mm();

#endif

  Log("'Hello World!' from Nanos-lite");

#ifdef __PAGE

  _Protect user_as;  // user process address space

  _protect(&user_as);

  entry = loader(&user_as);

  _switch(&user_as);

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
