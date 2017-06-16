#include "common.h"

uint32_t loader(void);

/* Uncomment the following macro after paging is enabling. */
//#define PAGE

#ifndef PAGE
static inline void relocate_myself() {
  const uintptr_t new_start = 0x800000;
  extern char _start, _end, rel_finish;
  void *start = &_start;
  void *end = &_end;
  memcpy((void *)new_start, start, end - start);
  asm volatile("jmp *%0;\n\t rel_finish:" : :"r"(&rel_finish - &_start + new_start));
}
#endif

int main() {
#ifndef PAGE
  relocate_myself();
#endif

  uint32_t entry = loader();
  ((void(*)(void))entry)();
}
