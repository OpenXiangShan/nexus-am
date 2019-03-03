#include <am.h>
#include <riscv32.h>
#include <klib.h>

extern char _heap_start;
extern char _heap_end;
extern int main();
void init_uartlite(void);
void uartlite_putchar(char ch);
void init_perfcnt(void);
void show_perfcnt(void);

_Area _heap = {
  .start = &_heap_start,
  .end = &_heap_end,
};

void _putc(char ch) {
  uartlite_putchar(ch);
}

void _halt(int code) {
  __asm__ volatile("mv a0, %0; .word 0x0005006b" : :"r"(code));
  printf("Exit with code = %d\n", code);

  // should not reach here
  while (1);
}

void _trm_init() {
  PerfCntSet t0, t1, res;
  init_uartlite();
  perfcnt_read(&t0);
  int ret = main();
  perfcnt_read(&t1);
  perfcnt_sub(&res, &t1, &t0);
  perfcnt_show(&res);
  _halt(ret);
}
