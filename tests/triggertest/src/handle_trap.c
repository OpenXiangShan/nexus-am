#include <am.h>
#include <klib.h>
#include <trigger.h>
#include <encoding.h>
#include <debug_defines.h>

extern void trap_entry(void);

void handle_trap(uintptr_t *sp) {
  printf("trapped with cause %lx at addr %lx\n", *(sp+32), *(sp+34));
  // assert
  *(sp+34) += (*(sp+34) & 0x10 ? 2 : 4);
}

void init_trap(void) {
  write_csr(mtvec, (uintptr_t) trap_entry);
}