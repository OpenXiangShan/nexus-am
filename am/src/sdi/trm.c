#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <nemu.h>

extern char _heap_start;
int main(const char *args);

_Area _heap = RANGE(&_heap_start, &_pmem_end);

void _trm_init() {
  extern const char __am_mainargs;
  int ret = main(&__am_mainargs);
  _halt(ret);
}

void _putc(char ch) {
  outb(SERIAL_PORT, ch);
}

void _halt(int code) {
  printf("Exit with code = %d\n", code);

#if defined(__ISA_X86__)
  asm volatile (".byte 0xd6" : :"a"(code));
#elif defined(__ISA_MIPS32__)
  asm volatile ("move $v0, %0; .word 0xf0000000" : :"r"(code));
#elif defined(__ISA_RISCV32__) || defined(__ISA_RISCV64__)
  asm volatile("mv a0, %0; .word 0x0000006b" : :"r"(code));
#else
#error unsupport ISA
#endif

  printf("Spinning\n");
  while (1);
}
