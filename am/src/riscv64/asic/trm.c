#include <am.h>
#include <riscv64.h>
#include <klib.h>

extern char _heap_start;
extern char _heap_end;
int main(const char *args);
void __am_init_uartlite(void);
void __am_uartlite_putchar(char ch);

_Area _heap = {
  .start = &_heap_start,
  .end = &_heap_end,
};

void _putc(char ch) {
  __am_uartlite_putchar(ch);
}

void _halt(int code) {
//  __asm__ volatile("mv a0, %0; .word 0x0005006b" : :"r"(code));

  // should not reach here during simulation
  printf("Exit with code = %d\n", code);

  // should not reach here on FPGA
  while (1);
}

#define L2_SIZE 128 * 1024
#define L2_CACHELINE_SIZE 64 // byte
#define L2_WAY 4
#define L2_SET (L2_SIZE / L2_CACHELINE_SIZE / L2_WAY)
#define L2_WAY_STEP (L2_SET * L2_CACHELINE_SIZE)

void _trm_init() {
  __am_init_uartlite();

  printf("Hello world! Build time: %s %s\n", __DATE__, __TIME__);
  volatile uint64_t *p = (void *)(uintptr_t)0x80100000;

#define macro(i) p[i] = (uint64_t)&p[i]
  macro(L2_WAY_STEP * 0 + 8 * 0);  // 0x80100000 <- 0x0000000080100000ull;
  macro(L2_WAY_STEP * 1 + 8 * 1);  // 0x80108008 <- 0x0000000080108008ull;
  macro(L2_WAY_STEP * 2 + 8 * 2);  // 0x80110010 <- 0x0000000080110010ull;
  macro(L2_WAY_STEP * 3 + 8 * 3);  // 0x80118018 <- 0x0000000080118018ull;
  macro(L2_WAY_STEP * 4 + 8 * 4);  // 0x80120020 <- 0x0000000080120020ull;
  macro(L2_WAY_STEP * 5 + 8 * 5);  // 0x80128028 <- 0x0000000080128028ull;
  macro(L2_WAY_STEP * 6 + 8 * 6);  // 0x80130030 <- 0x0000000080130030ull;
  macro(L2_WAY_STEP * 7 + 8 * 7);  // 0x80138038 <- 0x0000000080138038ull;

#define check(i) assert(p[i] == (uint64_t)&p[i])
  check(L2_WAY_STEP * 0 + 8 * 0);
  check(L2_WAY_STEP * 1 + 8 * 1);
  check(L2_WAY_STEP * 2 + 8 * 2);
  check(L2_WAY_STEP * 3 + 8 * 3);
  check(L2_WAY_STEP * 4 + 8 * 4);
  check(L2_WAY_STEP * 5 + 8 * 5);
  check(L2_WAY_STEP * 6 + 8 * 6);
  check(L2_WAY_STEP * 7 + 8 * 7);

  _halt(0);

  extern const char _mainargs;
  //char *args = &_mainargs;
  //strcpy(args, "h");
  int ret = main(&_mainargs);
  _halt(ret);
}
