#include <am.h>
#include <riscv32.h>
#include <klib.h>

extern char _heap_start;
extern char _heap_end;
int main();
void __am_init_uartlite(void);
void __am_uartlite_putchar(char ch);
void __am_init_perfcnt(void);
void __am_show_perfcnt(void);

_Area _heap = {
  .start = &_heap_start,
  .end = &_heap_end,
};

void _putc(char ch) {
  __am_uartlite_putchar(ch);
}

void _halt(int code) {
  __asm__ volatile("mv a0, %0; .word 0x0005006b" : :"r"(code));
  printf("Exit with code = %d\n", code);

  // should not reach here
  while (1);
}

void _trm_init() {
  PerfCntSet t0, t1, res;
  __am_init_uartlite();
  __am_perfcnt_read(&t0);
  int ret = main();
  __am_perfcnt_read(&t1);
  __am_perfcnt_sub(&res, &t1, &t0);
  __am_perfcnt_show(&res);
  _halt(ret);
}
