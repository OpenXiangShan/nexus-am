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

_Context *dummy_handler(_Event ev, _Context *c) {
  return NULL;
}

void _trm_init() {
  init_uartlite();
  _cte_init(dummy_handler);
  init_perfcnt();
  int ret = main();
  show_perfcnt();
  _halt(ret);
}
