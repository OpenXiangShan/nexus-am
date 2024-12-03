#include <am.h>
#include <riscv.h>
#include <klib.h>

extern char _heap_start;
extern char _pmem_end;
int main(const char *args);
void __am_init_uartlite(void);
void __am_uartlite_putchar(char ch);
void __am_init_16550(void);
void __am_16550_putchar(char ch);

_Area _heap = {
  .start = &_heap_start,
  .end = &_pmem_end,
};

void _putc(char ch) {
#ifdef UART16550
  __am_16550_putchar(ch);
#else
  __am_uartlite_putchar(ch);
#endif
}

void _halt(int code) {
  __asm__ volatile("mv a0, %0; .word 0x0005006b" : :"r"(code));

  // should not reach here during simulation
  printf("Exit with code = %d\n", code);

  // should not reach here on FPGA
  while (1);
}

void _trm_init() {
#ifdef UART16550
  __am_init_16550();
#else
  __am_init_uartlite();
#endif
  extern const char __am_mainargs;
  int ret = main(&__am_mainargs);
  _halt(ret);
}
